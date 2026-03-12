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
	Super::BeginPlay();
	// If HeroId is set (e.g. in Details when placed in level), load data from registry for testing.
	if (HeroId != NAME_None)
	{
		InitializeFromRegistry();
	}
}

void ARTSHeroCharacter::InitializeFromHeroRow(const FHeroRow& HeroRow, const FUnitRow& AsUnitRow)
{
	CachedHeroData = HeroRow;
	HeroId = HeroRow.HeroId;

	// Initialize base unit data first.
	InitializeFromUnitRow(AsUnitRow);

	// Command radius will be tuned later; for now we leave default from component.
}

bool ARTSHeroCharacter::InitializeFromRegistry()
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

