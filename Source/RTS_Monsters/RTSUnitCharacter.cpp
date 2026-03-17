#include "RTSUnitCharacter.h"

#include "RTSCommandAuthorityComponent.h"
#include "RTSDayNightSubsystem.h"
#include "RTSHeroCharacter.h"
#include "RTSSquadState.h"
#include "RTSSquadManagerSubsystem.h"
#include "RTSDataRegistry.h"
#include "RTSMoraleComponent.h"
#include "RTSOrderComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

static constexpr float CaptainCommandRadius = 1200.f;
static constexpr float HeroCommandRadius = 2500.f;
static constexpr float MoraleThresholdLow = 30.f;
static constexpr float MoraleThresholdCritical = 15.f;
static constexpr float MoraleSpeedMultiplierLow = 0.85f;
static constexpr float DetachedMoraleDrainPer5s = -1.f;
static constexpr float CasualtyMoralePenalty = -5.f;
static constexpr float CaptainDeathMoralePenalty = -10.f;
static constexpr float VampireNightDamageMultiplier = 1.1f;
static constexpr float VampireDayDamageMultiplier = 0.95f;
static constexpr float VampireNightMoveMultiplier = 1.05f;
static constexpr float VampireDayMoveMultiplier = 0.95f;

ARTSUnitCharacter::ARTSUnitCharacter()
	: Rank(1)
	, bIsCaptain(false)
	, CommandAuthorityComponent(nullptr)
{
	PrimaryActorTick.bCanEverTick = false;

	MoraleComponent = CreateDefaultSubobject<URTSMoraleComponent>(TEXT("MoraleComponent"));
	OrderComponent = CreateDefaultSubobject<URTSOrderComponent>(TEXT("OrderComponent"));

	// Ensure capsule blocks Visibility so unit can be selected by ray trace.
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}

	// Selection ring at feet (flat cylinder); visibility toggled in SetSelected.
	SelectionRingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectionRing"));
	SelectionRingMesh->SetupAttachment(RootComponent);
	SelectionRingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SelectionRingMesh->SetVisibility(false);
	if (UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder")))
	{
		SelectionRingMesh->SetStaticMesh(Cylinder);
		SelectionRingMesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 0.02f));
	}

	// Body mesh: visible capsule so spawned units are always visible (Blueprint can override/hide).
	BodyMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMeshComponent->SetupAttachment(GetCapsuleComponent());
	BodyMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder")))
	{
		BodyMeshComponent->SetStaticMesh(Cylinder);
		BodyMeshComponent->SetRelativeScale3D(FVector(0.5f, 0.5f, 1.8f));  // Capsule-like (radius ~25, height ~180)
	}
}

void ARTSUnitCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!PersistentUnitGuid.IsValid())
	{
		PersistentUnitGuid = FGuid::NewGuid();
	}

	// Ensure capsule blocks Visibility (Blueprint may override constructor; reapply here).
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}

	// Hide BodyMesh if Blueprint has a SkeletalMesh (avoid double body).
	if (BodyMeshComponent && GetMesh() && GetMesh()->GetSkeletalMeshAsset())
	{
		BodyMeshComponent->SetVisibility(false);
	}

	// Position selection ring at feet.
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		if (SelectionRingMesh)
		{
			SelectionRingMesh->SetRelativeLocation(FVector(0.f, 0.f, -HalfHeight));
		}
	}

	if (bInitializeFromRegistry && UnitId != NAME_None)
	{
		InitializeFromRegistry();
	}
	else if (!bInitializeFromRegistry)
	{
		// Use only the values you set in Cached Unit Data (no table load).
		InitializeFromUnitRow(CachedUnitData);
	}

	// Apply per-instance overrides so this unit can differ from the DataTable (e.g. same Militia row but faster).
	if (OverrideMoveSpeed > 0.f)
	{
		CachedUnitData.MoveSpeed = OverrideMoveSpeed;
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = OverrideMoveSpeed;
		}
	}
	if (OverrideHP > 0.f)
	{
		CachedUnitData.HP = OverrideHP;
	}
	if (OverrideDamage > 0.f)
	{
		CachedUnitData.Damage = OverrideDamage;
	}

	// P3: Register with squad manager (add to faction squad).
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (URTSSquadManagerSubsystem* Mgr = GI->GetSubsystem<URTSSquadManagerSubsystem>())
			{
				Mgr->AddUnitToFactionSquad(this);
			}
		}
	}

	// P3: Morale and detached timers (skip for Heroes – they are always authority).
	if (!Cast<ARTSHeroCharacter>(this))
	{
		if (UWorld* World = GetWorld())
		{
			FTimerManager& TM = World->GetTimerManager();
			TM.SetTimer(TimerHandleMoraleUpdate, this, &ARTSUnitCharacter::UpdateMoraleEffects, 1.f, true, 0.5f);
			TM.SetTimer(TimerHandleDetachedAndDrain, this, &ARTSUnitCharacter::UpdateDetachedAndDrain, 5.f, true, 2.f);
		}
	}
}

void ARTSUnitCharacter::Destroyed()
{
	if (URTSSquadState* Squad = SquadReference.Get())
	{
		// Remove this unit first so morale penalties don't include the dying unit's own morale.
		Squad->RemoveMember(this);
		if (bIsCaptain)
		{
			Squad->ApplyMoraleDeltaToAll(CaptainDeathMoralePenalty);
		}
		Squad->ApplyMoraleDeltaToAll(CasualtyMoralePenalty);
	}
	Super::Destroyed();
}

void ARTSUnitCharacter::SetSquad(URTSSquadState* Squad)
{
	SquadReference = Squad;
}

void ARTSUnitCharacter::InitializeFromUnitRow(const FUnitRow& UnitRow)
{
	CachedUnitData = UnitRow;
	FactionId = UnitRow.Faction;
	UnitId = UnitRow.UnitId;
	Rank = UnitRow.Rank;
	Level = FMath::Max(1, UnitRow.Level);

	if (MoraleComponent)
	{
		MoraleComponent->BaseMorale = UnitRow.MoraleBase;
		MoraleComponent->SetMorale(UnitRow.MoraleBase);
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = UnitRow.MoveSpeed;
	}
}

bool ARTSUnitCharacter::InitializeFromRegistry()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}
	URTSDataRegistry* Registry = World->GetGameInstance()->GetSubsystem<URTSDataRegistry>();
	if (!Registry)
	{
		return false;
	}
	FUnitRow Row;
	if (!Registry->GetUnitRow(UnitId, Row))
	{
		return false;
	}
	InitializeFromUnitRow(Row);
	return true;
}

void ARTSUnitCharacter::SetCurrentOrder(ERTSOrderType OrderType, const FRTSOrderPayload& Payload)
{
	// P3: Detached units accept only defensive behavior (no new Attack).
	if (bIsDetached && OrderType == ERTSOrderType::Attack)
	{
		return;
	}
	if (OrderComponent)
	{
		OrderComponent->SetCurrentOrder(OrderType, Payload);
	}
}

bool ARTSUnitCharacter::TryPromoteToCaptain(const ARTSUnitCharacter* Issuer)
{
	if (!Issuer || bIsCaptain)
	{
		return false;
	}
	if (Cast<ARTSHeroCharacter>(this))
	{
		return false; // Heroes are not promoted to Captain.
	}
	if (FactionId != Issuer->FactionId)
	{
		return false;
	}
	if (Rank < 3)
	{
		return false;
	}

	URTSCommandAuthorityComponent* Authority = Cast<URTSCommandAuthorityComponent>(AddComponentByClass(URTSCommandAuthorityComponent::StaticClass(), false, FTransform::Identity, false));
	if (Authority)
	{
		Authority->CommandRadius = CaptainCommandRadius;
		Authority->bAuthorityEnabled = true;
		CommandAuthorityComponent = Authority;
		bIsCaptain = true;
		if (URTSSquadState* Squad = SquadReference.Get())
		{
			Squad->SetCaptain(this);
		}
		return true;
	}
	return false;
}

void ARTSUnitCharacter::SetSelected(bool bSelected)
{
	if (SelectionRingMesh)
	{
		SelectionRingMesh->SetVisibility(bSelected);
	}
	if (USkeletalMeshComponent* SkelMesh = GetMesh())
	{
		SkelMesh->SetRenderCustomDepth(bSelected);
		SkelMesh->SetCustomDepthStencilValue(bSelected ? 1 : 0);
	}
	// Body mesh (for spawned units without SkeletalMesh) – selection outline
	if (BodyMeshComponent)
	{
		BodyMeshComponent->SetRenderCustomDepth(bSelected);
		BodyMeshComponent->SetCustomDepthStencilValue(bSelected ? 1 : 0);
	}
}

float ARTSUnitCharacter::GetOrderResponsivenessMultiplier() const
{
	if (!MoraleComponent)
	{
		return 1.f;
	}
	return MoraleComponent->CurrentMorale < MoraleThresholdLow ? MoraleSpeedMultiplierLow : 1.f;
}

float ARTSUnitCharacter::GetDayNightDamageMultiplier() const
{
	if (FactionId != EFactionId::Vampires) return 1.f;
	UWorld* World = GetWorld();
	if (!World) return 1.f;
	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return 1.f;
	URTSDayNightSubsystem* DayNight = GI->GetSubsystem<URTSDayNightSubsystem>();
	if (!DayNight) return 1.f;
	return DayNight->IsNight() ? VampireNightDamageMultiplier : VampireDayDamageMultiplier;
}

float ARTSUnitCharacter::GetDayNightMoveSpeedMultiplier() const
{
	if (FactionId != EFactionId::Vampires) return 1.f;
	UWorld* World = GetWorld();
	if (!World) return 1.f;
	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return 1.f;
	URTSDayNightSubsystem* DayNight = GI->GetSubsystem<URTSDayNightSubsystem>();
	if (!DayNight) return 1.f;
	return DayNight->IsNight() ? VampireNightMoveMultiplier : VampireDayMoveMultiplier;
}

void ARTSUnitCharacter::UpdateMoraleEffects()
{
	if (!MoraleComponent || !GetCharacterMovement())
	{
		return;
	}
	const float BaseSpeed = CachedUnitData.MoveSpeed > 0.f ? CachedUnitData.MoveSpeed : 420.f;
	const float MoraleMult = MoraleComponent->CurrentMorale < MoraleThresholdLow ? MoraleSpeedMultiplierLow : 1.f;
	const float DayNightMult = GetDayNightMoveSpeedMultiplier();
	GetCharacterMovement()->MaxWalkSpeed = BaseSpeed * MoraleMult * DayNightMult;
}

void ARTSUnitCharacter::UpdateDetachedAndDrain()
{
	if (Cast<ARTSHeroCharacter>(this))
	{
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const FVector MyLoc = GetActorLocation();
	bool bAuthorityInRange = false;

	// O(H) scan: only iterate Heroes (TActorIterator<ARTSHeroCharacter>), not all N units.
	// Heroes are typically 1-3 per session; this reduces cost from O(N²) to O(N*H) ≈ O(N).
	for (TActorIterator<ARTSHeroCharacter> HeroIt(World); HeroIt; ++HeroIt)
	{
		ARTSHeroCharacter* Hero = *HeroIt;
		if (!Hero || !IsValid(Hero) || Hero->FactionId != FactionId) continue;
		if (FVector::Dist(MyLoc, Hero->GetActorLocation()) <= HeroCommandRadius)
		{
			bAuthorityInRange = true;
			break;
		}
	}

	// If no Hero in range, check Captains via SquadState (O(1) lookup per squad).
	if (!bAuthorityInRange)
	{
		if (URTSSquadState* Squad = SquadReference.Get())
		{
			if (ARTSUnitCharacter* Captain = Squad->GetCaptain())
			{
				if (Captain != this && IsValid(Captain) && Captain->bIsCaptain)
				{
					if (FVector::Dist(MyLoc, Captain->GetActorLocation()) <= CaptainCommandRadius)
					{
						bAuthorityInRange = true;
					}
				}
			}
		}
	}

	bIsDetached = !bAuthorityInRange;
	if (bIsDetached && MoraleComponent)
	{
		MoraleComponent->ApplyMoraleDelta(DetachedMoraleDrainPer5s);
	}
	if (MoraleComponent && MoraleComponent->CurrentMorale < MoraleThresholdCritical)
	{
		if (FMath::FRand() < 0.2f)
		{
			TickLowMoraleAutoRetreat();
		}
	}
}

void ARTSUnitCharacter::TickLowMoraleAutoRetreat()
{
	if (OrderComponent)
	{
		OrderComponent->ClearOrder(false);
	}
	FRTSOrderPayload Payload;
	Payload.MoveDestination = GetActorLocation() - 400.f * GetActorForwardVector();
	SetCurrentOrder(ERTSOrderType::Move, Payload);
}

float ARTSUnitCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	// Single Damage Authority: all combat damage must go through URTSCombatManagerSubsystem.
	// Direct TakeDamage calls are blocked to prevent double-damage paths.
	// If you need environmental damage (traps, hazards) use a separate non-combat system.
	UE_LOG(LogTemp, Warning, TEXT("[RTS] BLOCKED TakeDamage on %s (%.1f dmg). Use CombatManager only."),
		*GetName(), DamageAmount);
	return 0.f;
}

void ARTSUnitCharacter::SanitizeOrdersForDetached()
{
	// PostLoadFixup: detached units must not have Attack orders (COMBAT_CONTRACT + Detached rule).
	if (bIsDetached && OrderComponent && OrderComponent->CurrentOrderType == ERTSOrderType::Attack)
	{
		UE_LOG(LogTemp, Log, TEXT("[RTS] PostLoadFixup: %s is detached, sanitizing Attack -> None."), *GetName());
		OrderComponent->ClearOrder(false);
	}
}

