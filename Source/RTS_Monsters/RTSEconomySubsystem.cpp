#include "RTSEconomySubsystem.h"

#include "RTSRegionVolume.h"
#include "RTSHeroCharacter.h"
#include "RTSUnitCharacter.h"
#include "RTSHumanNPC.h"
#include "RTSDataRegistry.h"
#include "RTSSquadManagerSubsystem.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"

static constexpr float CaptureRangeForRecruit = 600.f;  // Hero can capture NPC within this range

void URTSEconomySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MoneyByFaction.Add(EFactionId::Humans, 100);  // Starting Money so Humans can recruit before control gains
	MoneyByFaction.Add(EFactionId::Vampires, 0);
	MoneyByFaction.Add(EFactionId::Werewolves, 0);

	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (World)
	{
		World->GetTimerManager().SetTimer(EconomyTickHandle, this, &URTSEconomySubsystem::TickEconomy,
			EconomyTickIntervalSeconds, true);
	}
}

void URTSEconomySubsystem::Deinitialize()
{
	if (UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr)
	{
		World->GetTimerManager().ClearTimer(EconomyTickHandle);
	}
	Super::Deinitialize();
}

void URTSEconomySubsystem::AddMoney(EFactionId Faction, int32 Amount)
{
	if (int32* Ptr = MoneyByFaction.Find(Faction))
	{
		*Ptr = FMath::Max(0, *Ptr + Amount);
	}
}

bool URTSEconomySubsystem::SpendMoney(EFactionId Faction, int32 Amount)
{
	int32* Ptr = MoneyByFaction.Find(Faction);
	if (!Ptr || *Ptr < Amount)
	{
		return false;
	}
	*Ptr -= Amount;
	return true;
}

int32 URTSEconomySubsystem::GetMoney(EFactionId Faction) const
{
	const int32* Ptr = MoneyByFaction.Find(Faction);
	return Ptr ? *Ptr : 0;
}

bool URTSEconomySubsystem::CanAffordRecruit(EFactionId Faction, FName UnitId, ARTSRegionVolume* Region) const
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return false;

	URTSDataRegistry* Registry = GI->GetSubsystem<URTSDataRegistry>();
	if (!Registry) return false;

	FUnitRow Row;
	if (!Registry->GetUnitRow(UnitId, Row))
	{
		return false;
	}

	if (Faction == EFactionId::Humans)
	{
		return Row.RecruitCostMoney > 0 && GetMoney(Faction) >= Row.RecruitCostMoney;
	}

	// Vampires / Werewolves: need Region and Population
	if (Faction == EFactionId::Vampires || Faction == EFactionId::Werewolves)
	{
		return Row.ConvertPopulationCost > 0 && Region && Region->GetPopulation() >= Row.ConvertPopulationCost;
	}

	return false;
}

ARTSUnitCharacter* URTSEconomySubsystem::TryRecruitUnit(ARTSHeroCharacter* Hero, FName UnitId)
{
	if (!Hero || !IsValid(Hero))
	{
		return nullptr;
	}

	UWorld* World = Hero->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		return nullptr;
	}

	URTSDataRegistry* Registry = GI->GetSubsystem<URTSDataRegistry>();
	if (!Registry)
	{
		return nullptr;
	}

	ARTSRegionVolume* Region = ARTSRegionVolume::GetRegionAtLocation(Hero, Hero->GetActorLocation());
	if (!Region)
	{
		return nullptr;
	}

	FUnitRow Row;
	if (!Registry->GetUnitRow(UnitId, Row))
	{
		return nullptr;
	}

	if (Row.Faction != Hero->FactionId)
	{
		return nullptr;
	}

	// Phase 3: Vampires/Werewolves – try to capture physical ARTSHumanNPC first
	if (Hero->FactionId == EFactionId::Vampires || Hero->FactionId == EFactionId::Werewolves)
	{
		ARTSHumanNPC* CapturedNPC = nullptr;
		const FVector HeroLoc = Hero->GetActorLocation();
		for (TActorIterator<ARTSHumanNPC> It(World); It; ++It)
		{
			ARTSHumanNPC* NPC = *It;
			if (!IsValid(NPC) || !NPC->CanBeCaptured())
			{
				continue;
			}
			// Must be in same region as Hero (use HomeRegion if set, else check point-in-region)
			const bool bInRegion = (NPC->HomeRegion.Get() == Region) || (Region && Region->IsPointInRegion(NPC->GetActorLocation()));
			if (!bInRegion)
			{
				continue;
			}
			const float DistSq = FVector::DistSquared(HeroLoc, NPC->GetActorLocation());
			if (DistSq <= FMath::Square(CaptureRangeForRecruit))
			{
				CapturedNPC = NPC;
				break;
			}
		}

		if (CapturedNPC && CapturedNPC->TryCapture())
		{
			// Spawn near Hero (not at NPC location – avoids capsule overlap and Destroy() issues)
			const FVector SpawnLoc = HeroLoc + (Hero->GetActorForwardVector() * 150.f);
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			UClass* UnitClass = ARTSUnitCharacter::StaticClass();
			if (UClass* Loaded = DefaultUnitClassPath.TryLoadClass<ARTSUnitCharacter>())
			{
				UnitClass = Loaded;
			}
			ARTSUnitCharacter* NewUnit = World->SpawnActor<ARTSUnitCharacter>(UnitClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
			if (NewUnit)
			{
				NewUnit->UnitId = UnitId;
				NewUnit->FactionId = Hero->FactionId;
				NewUnit->bInitializeFromRegistry = true;
				NewUnit->InitializeFromRegistry();
				if (URTSSquadManagerSubsystem* SquadMgr = GI->GetSubsystem<URTSSquadManagerSubsystem>())
				{
					SquadMgr->AddUnitToFactionSquad(NewUnit);
				}
				CapturedNPC->Destroy();
				return NewUnit;
			}
			CapturedNPC->SetNPCState(EHumanNPCState::Idle);  // Revert capture on spawn failure
		}
	}

	if (Hero->FactionId == EFactionId::Humans)
	{
		if (!CanAffordRecruit(Hero->FactionId, UnitId, Region))
		{
			return nullptr;
		}
		if (!SpendMoney(Hero->FactionId, Row.RecruitCostMoney))
		{
			return nullptr;
		}
	}
	else
	{
		// Vampires/Werewolves: no physical NPC found – use abstract Population
		if (!CanAffordRecruit(Hero->FactionId, UnitId, Region))
		{
			return nullptr;
		}
		Region->SetPopulation(FMath::Max(0, Region->GetPopulation() - Row.ConvertPopulationCost));
	}

	// Spawn unit in front of Hero
	const FVector SpawnLoc = Hero->GetActorLocation() + Hero->GetActorForwardVector() * 150.f;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	UClass* UnitClass = ARTSUnitCharacter::StaticClass();
	if (UClass* Loaded = DefaultUnitClassPath.TryLoadClass<ARTSUnitCharacter>())
	{
		UnitClass = Loaded;
	}
	ARTSUnitCharacter* NewUnit = World->SpawnActor<ARTSUnitCharacter>(UnitClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (!NewUnit)
	{
		// Refund on failure
		if (Hero->FactionId == EFactionId::Humans)
		{
			AddMoney(Hero->FactionId, Row.RecruitCostMoney);
		}
		else
		{
			Region->SetPopulation(Region->GetPopulation() + Row.ConvertPopulationCost);
		}
		return nullptr;
	}

	NewUnit->UnitId = UnitId;
	NewUnit->FactionId = Hero->FactionId;
	NewUnit->bInitializeFromRegistry = true;
	NewUnit->InitializeFromRegistry();

	// Add to squad (P3)
	if (URTSSquadManagerSubsystem* SquadMgr = GI->GetSubsystem<URTSSquadManagerSubsystem>())
	{
		SquadMgr->AddUnitToFactionSquad(NewUnit);
	}

	return NewUnit;
}

void URTSEconomySubsystem::TickEconomy()
{
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		return;
	}

	// Humans gain Money from regions where Hero is present and faction has control
	for (TActorIterator<ARTSRegionVolume> It(World); It; ++It)
	{
		ARTSRegionVolume* R = *It;
		if (!IsValid(R))
		{
			continue;
		}

		for (int32 F = 0; F < 3; ++F)
		{
			const EFactionId Faction = static_cast<EFactionId>(F);
			if (Faction != EFactionId::Humans)
			{
				continue; // Vampires/Werewolves don't gain Money
			}

			if (!R->HasHeroOfFaction(Faction))
			{
				continue;
			}

			const int32 ControlLevel = R->GetControlLevelForFaction(Faction);
			// Allow Money gain even at control 0 when Hero in region (reduced rate)
			const float Rate = (ControlLevel > 0)
				? (MoneyBaseRate + R->GetPopulation() * MoneyPerPopulation + R->GetStability() * MoneyPerStability + ControlLevel * MoneyPerControlLevel)
				: (MoneyBaseRate * 0.5f);

			const int32 Amount = FMath::RoundToInt(Rate * EconomyTickIntervalSeconds);
			if (Amount > 0)
			{
				AddMoney(Faction, Amount);
			}
		}
	}
}
