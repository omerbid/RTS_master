#include "RTSHeroCharacter.h"

#include "RTSCommandAuthorityComponent.h"
#include "RTSDataRegistry.h"
#include "RTSSecureRegionComponent.h"
#include "RTSVictorySubsystem.h"

ARTSHeroCharacter::ARTSHeroCharacter()
{
	CommandAuthorityComponent = CreateDefaultSubobject<URTSCommandAuthorityComponent>(TEXT("CommandAuthority"));
	SecureRegionComponent = CreateDefaultSubobject<URTSSecureRegionComponent>(TEXT("SecureRegion"));
}

bool ARTSHeroCharacter::TryStartSecureRegion()
{
	return SecureRegionComponent ? SecureRegionComponent->StartSecureRegion() : false;
}

void ARTSHeroCharacter::BeginPlay()
{
	// BUG-08 fix: suppress base-class registry init so hero data is loaded exactly once.
	// ARTSUnitCharacter::BeginPlay checks bInitializeFromRegistry; if we leave it true AND
	// HeroId is set, both the unit-level and hero-level init would run (double morale + stat reset).
	// Heroes always initialize via the hero path (InitializeFromRegistry below), so we disable
	// the unit path temporarily before calling Super.
	const bool bWasUnitRegistryInit = bInitializeFromRegistry;
	bInitializeFromRegistry = false;

	Super::BeginPlay();

	bInitializeFromRegistry = bWasUnitRegistryInit;

	// Hero-specific init: loads HeroRow + UnitRow and calls InitializeFromHeroRow (single path).
	if (HeroId != NAME_None)
	{
		if (!InitializeFromRegistry())
		{
			UE_LOG(LogTemp, Warning, TEXT("[RTS|Hero] BeginPlay: InitializeFromRegistry failed for HeroId=%s. Check DataRegistry."), *HeroId.ToString());
		}
	}
}

void ARTSHeroCharacter::InitializeFromHeroRow(const FHeroRow& HeroRow, const FUnitRow& AsUnitRow)
{
	CachedHeroData = HeroRow;
	HeroId = HeroRow.HeroId;

	// Initialize base unit data first.
	InitializeFromUnitRow(AsUnitRow);

	if (CommandAuthorityComponent && HeroRow.CommandRadius > 0.f)
	{
		CommandAuthorityComponent->CommandRadius = HeroRow.CommandRadius;
	}
}

bool ARTSHeroCharacter::InitializeFromRegistry()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}
	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return false;
	URTSDataRegistry* Registry = GI->GetSubsystem<URTSDataRegistry>();
	if (!Registry)
	{
		return false;
	}
	FHeroRow HeroRow;
	if (!Registry->GetHeroRow(HeroId, HeroRow))
	{
		return false;
	}
	FUnitRow UnitRow;
	if (!Registry->GetUnitRowForHero(HeroRow, UnitRow))
	{
		return false;
	}
	InitializeFromHeroRow(HeroRow, UnitRow);
	return true;
}

void ARTSHeroCharacter::IssueOrderToUnitsInRange(const TArray<ARTSUnitCharacter*>& Units)
{
	if (!CommandAuthorityComponent)
	{
		return;
	}

	for (ARTSUnitCharacter* Unit : Units)
	{
		if (CommandAuthorityComponent->CanIssueOrderToUnit(Unit))
		{
			// P0: only validation stub. Actual order logic (move/attack) will be implemented in P1.
			// This function exists to prove authority checks can be called from Hero.
		}
	}
}

void ARTSHeroCharacter::Destroyed()
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (URTSVictorySubsystem* Victory = GI->GetSubsystem<URTSVictorySubsystem>())
			{
				Victory->NotifyHeroDeath(this);
			}
		}
	}
	Super::Destroyed();
}

