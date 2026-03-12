#include "RTSUnitCharacter.h"

#include "RTSCommandAuthorityComponent.h"
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
}

void ARTSUnitCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Ensure capsule blocks Visibility (Blueprint may override constructor; reapply here).
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
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
		if (bIsCaptain)
		{
			Squad->ApplyMoraleDeltaToAll(CaptainDeathMoralePenalty);
		}
		Squad->RemoveMember(this);
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
}

float ARTSUnitCharacter::GetOrderResponsivenessMultiplier() const
{
	if (!MoraleComponent)
	{
		return 1.f;
	}
	return MoraleComponent->CurrentMorale < MoraleThresholdLow ? MoraleSpeedMultiplierLow : 1.f;
}

void ARTSUnitCharacter::UpdateMoraleEffects()
{
	if (!MoraleComponent || !GetCharacterMovement())
	{
		return;
	}
	const float BaseSpeed = CachedUnitData.MoveSpeed > 0.f ? CachedUnitData.MoveSpeed : 420.f;
	const float Mult = MoraleComponent->CurrentMorale < MoraleThresholdLow ? MoraleSpeedMultiplierLow : 1.f;
	GetCharacterMovement()->MaxWalkSpeed = BaseSpeed * Mult;
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
	for (TActorIterator<ARTSUnitCharacter> It(World); It; ++It)
	{
		ARTSUnitCharacter* Other = *It;
		if (!Other || Other->FactionId != FactionId)
		{
			continue;
		}
		const float Dist = FVector::Dist(MyLoc, Other->GetActorLocation());
		if (ARTSHeroCharacter* Hero = Cast<ARTSHeroCharacter>(Other))
		{
			if (Dist <= HeroCommandRadius)
			{
				bAuthorityInRange = true;
				break;
			}
		}
		else if (Other->bIsCaptain && Other->CommandAuthorityComponent)
		{
			if (Dist <= CaptainCommandRadius)
			{
				bAuthorityInRange = true;
				break;
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

