#include "RTSVictorySubsystem.h"

#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "RTSHeroCharacter.h"
#include "RTSRegionVolume.h"
#include "RTSDataRegistry.h"
#include "RTSSquadManagerSubsystem.h"
#include "RTSSquadState.h"
#include "RTSUnitCharacter.h"

void URTSVictorySubsystem::NotifyControlReachedFive(ARTSRegionVolume* Region, EFactionId Faction)
{
	if (GameResult != ERTSGameResult::Playing) return;
	UE_LOG(LogTemp, Log, TEXT("[RTS|Victory] Game WON by Faction=%d via region control."), (int32)Faction);
	GameResult = ERTSGameResult::Won;
	WinningFaction = Faction;
	OnGameWon.Broadcast(Faction);
}

void URTSVictorySubsystem::NotifyHeroDeath(ARTSHeroCharacter* Hero)
{
	if (!Hero || GameResult != ERTSGameResult::Playing) return;
	UWorld* World = Hero->GetWorld();
	if (!World) return;

	const EFactionId Faction = Hero->FactionId;
	const FName HeroIdToRespawn = Hero->HeroId;

	UE_LOG(LogTemp, Log, TEXT("[RTS|Victory] HeroDeath: %s (Faction=%d). Checking respawn eligibility."),
		*HeroIdToRespawn.ToString(), (int32)Faction);

	// P5 GDD: If faction has at least one region with control >= 3, start respawn flow instead of lose.
	if (HasRegionWithControlAtLeast3(World, Faction))
	{
		ApplyMoraleShockToSquadsNear(World, Hero->GetActorLocation(), Faction);
		StartRespawnTimer(Faction, HeroIdToRespawn);
		return;
	}

	CheckLoseCondition(World, Faction, Hero);
}

void URTSVictorySubsystem::CheckLoseCondition(UWorld* World, EFactionId Faction, ARTSHeroCharacter* ExcludeDyingHero)
{
	if (GameResult != ERTSGameResult::Playing) return;

	// Any living Hero of this faction (excluding the hero currently firing Destroyed(), still in world)?
	bool bHasLivingHero = false;
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(World, ARTSHeroCharacter::StaticClass(), Found);
	for (AActor* A : Found)
	{
		if (!IsValid(A) || A == ExcludeDyingHero) continue;
		ARTSHeroCharacter* H = Cast<ARTSHeroCharacter>(A);
		if (H && H->FactionId == Faction)
		{
			bHasLivingHero = true;
			break;
		}
	}
	if (bHasLivingHero) return;

	// Any region with control >= 3 for this faction?
	bool bHasRegionWithControl3 = false;
	for (TActorIterator<ARTSRegionVolume> It(World); It; ++It)
	{
		ARTSRegionVolume* R = *It;
		if (IsValid(R) && R->GetControlLevelForFaction(Faction) >= 3)
		{
			bHasRegionWithControl3 = true;
			break;
		}
	}
	if (bHasRegionWithControl3) return;

	// GDD: no living hero and no region >= 3 -> lose
	UE_LOG(LogTemp, Log, TEXT("[RTS|Victory] Game LOST by Faction=%d (no living hero, no region>=3)."), (int32)Faction);
	GameResult = ERTSGameResult::Lost;
	LosingFaction = Faction;
	OnGameLost.Broadcast(Faction);
}

bool URTSVictorySubsystem::HasRegionWithControlAtLeast3(UWorld* World, EFactionId Faction) const
{
	if (!World) return false;
	for (TActorIterator<ARTSRegionVolume> It(World); It; ++It)
	{
		ARTSRegionVolume* R = *It;
		if (IsValid(R) && R->GetControlLevelForFaction(Faction) >= 3)
		{
			return true;
		}
	}
	return false;
}

void URTSVictorySubsystem::ApplyMoraleShockToSquadsNear(UWorld* World, FVector Location, EFactionId Faction)
{
	if (!World || MoraleShockDelta == 0.f) return;
	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return;
	URTSSquadManagerSubsystem* SquadMgr = GI->GetSubsystem<URTSSquadManagerSubsystem>();
	if (!SquadMgr) return;

	const float RadiusSq = FMath::Square(MoraleShockRadius);
	// Apply to same faction's squad if any member is in range (GDD: death causes morale shock to nearby squads).
	URTSSquadState* Squad = SquadMgr->GetOrCreateSquadForFaction(Faction);
	if (Squad)
	{
		TArray<ARTSUnitCharacter*> Members = Squad->GetMembers();
		for (ARTSUnitCharacter* U : Members)
		{
			if (U && IsValid(U))
			{
				if (FVector::DistSquared(U->GetActorLocation(), Location) <= RadiusSq)
				{
					Squad->ApplyMoraleDeltaToAll(MoraleShockDelta);
					break;
				}
			}
		}
	}
}

void URTSVictorySubsystem::StartRespawnTimer(EFactionId Faction, FName HeroIdToRespawn)
{
	UWorld* World = GetWorld();
	if (!World) return;

	RespawnStateByFaction.Add(Faction, ERTSRespawnState::WaitingForTimer);
	RespawnHeroIdByFaction.Add(Faction, HeroIdToRespawn);

	FTimerManager& TM = World->GetTimerManager();
	TM.ClearTimer(RespawnTimerHandleByFaction.FindOrAdd(Faction));

	if (Faction == EFactionId::Humans)
		TM.SetTimer(RespawnTimerHandleByFaction[Faction], this, &URTSVictorySubsystem::OnRespawnTimerExpired_Humans, RespawnTimerSeconds, false);
	else if (Faction == EFactionId::Vampires)
		TM.SetTimer(RespawnTimerHandleByFaction[Faction], this, &URTSVictorySubsystem::OnRespawnTimerExpired_Vampires, RespawnTimerSeconds, false);
	else
		TM.SetTimer(RespawnTimerHandleByFaction[Faction], this, &URTSVictorySubsystem::OnRespawnTimerExpired_Werewolves, RespawnTimerSeconds, false);
}

void URTSVictorySubsystem::OnRespawnTimerExpired(EFactionId Faction)
{
	RespawnStateByFaction.Add(Faction, ERTSRespawnState::RitualAvailable);
	OnRespawnRitualAvailable.Broadcast(Faction);
}

ERTSRespawnState URTSVictorySubsystem::GetRespawnState(EFactionId Faction) const
{
	const ERTSRespawnState* State = RespawnStateByFaction.Find(Faction);
	return State ? *State : ERTSRespawnState::None;
}

bool URTSVictorySubsystem::CanPerformRitual(EFactionId Faction) const
{
	return GetRespawnState(Faction) == ERTSRespawnState::RitualAvailable;
}

bool URTSVictorySubsystem::GetFactionWithRitualAvailable(EFactionId& OutFaction) const
{
	for (const auto& Pair : RespawnStateByFaction)
	{
		if (Pair.Value == ERTSRespawnState::RitualAvailable)
		{
			OutFaction = Pair.Key;
			return true;
		}
	}
	return false;
}

bool URTSVictorySubsystem::StartRitualChannel(EFactionId Faction)
{
	if (GetRespawnState(Faction) != ERTSRespawnState::RitualAvailable) return false;

	UWorld* World = GetWorld();
	if (!World) return false;

	RespawnStateByFaction.Add(Faction, ERTSRespawnState::ChannelingRitual);

	FTimerManager& TM = World->GetTimerManager();
	TM.ClearTimer(RitualChannelHandleByFaction.FindOrAdd(Faction));

	if (Faction == EFactionId::Humans)
		TM.SetTimer(RitualChannelHandleByFaction[Faction], this, &URTSVictorySubsystem::OnRitualChannelComplete_Humans, RitualChannelSeconds, false);
	else if (Faction == EFactionId::Vampires)
		TM.SetTimer(RitualChannelHandleByFaction[Faction], this, &URTSVictorySubsystem::OnRitualChannelComplete_Vampires, RitualChannelSeconds, false);
	else
		TM.SetTimer(RitualChannelHandleByFaction[Faction], this, &URTSVictorySubsystem::OnRitualChannelComplete_Werewolves, RitualChannelSeconds, false);

	return true;
}

void URTSVictorySubsystem::CancelRitualChannel(EFactionId Faction)
{
	if (GetRespawnState(Faction) != ERTSRespawnState::ChannelingRitual) return;

	UWorld* World = GetWorld();
	if (World)
	{
		if (FTimerHandle* Handle = RitualChannelHandleByFaction.Find(Faction))
		{
			World->GetTimerManager().ClearTimer(*Handle);
		}
	}
	RespawnStateByFaction.Add(Faction, ERTSRespawnState::RitualAvailable);
}

void URTSVictorySubsystem::OnRitualChannelComplete(EFactionId Faction)
{
	FName HeroId = RespawnHeroIdByFaction.FindRef(Faction);

	UWorld* World = GetWorld();
	if (!World)
	{
		RespawnStateByFaction.Add(Faction, ERTSRespawnState::None);
		RespawnHeroIdByFaction.Add(Faction, NAME_None);
		return;
	}

	ARTSHeroCharacter* NewHero = SpawnHeroInBestRegion(World, Faction, HeroId);
	if (NewHero)
	{
		RespawnStateByFaction.Add(Faction, ERTSRespawnState::None);
		RespawnHeroIdByFaction.Add(Faction, NAME_None);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 4.f, FColor::Cyan,
				FString::Printf(TEXT("[RTS] Hero respawned: %s"), *HeroId.ToString()));
		}
	}
	else
	{
		// Fallback: no valid region (e.g. all contested or control lost). Stay RitualAvailable so player can retry.
		RespawnStateByFaction.Add(Faction, ERTSRespawnState::RitualAvailable);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.f, FColor::Yellow,
				TEXT("[RTS] Hero respawn failed: no region with control >= 3. Ritual still available."));
		}
	}
}

ARTSHeroCharacter* URTSVictorySubsystem::SpawnHeroInBestRegion(UWorld* World, EFactionId Faction, FName HeroId)
{
	if (!World || HeroId == NAME_None) return nullptr;

	// GDD: spawn in region with highest control for this faction (>= 3).
	ARTSRegionVolume* BestRegion = nullptr;
	int32 BestControl = 2;
	for (TActorIterator<ARTSRegionVolume> It(World); It; ++It)
	{
		ARTSRegionVolume* R = *It;
		if (!IsValid(R)) continue;
		const int32 Control = R->GetControlLevelForFaction(Faction);
		if (Control >= 3 && (BestRegion == nullptr || Control > BestControl))
		{
			BestControl = Control;
			BestRegion = R;
		}
	}
	if (!BestRegion) return nullptr;

	UClass* HeroClass = HeroRespawnClass.Get();
	if (!HeroClass) HeroClass = ARTSHeroCharacter::StaticClass();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	// Use designer-placed HeroSpawnOffset to avoid spawning inside terrain (BUG-18 fix).
	FVector SpawnLoc = BestRegion->GetHeroSpawnLocation();
	FRotator SpawnRot = FRotator::ZeroRotator;

	ARTSHeroCharacter* NewHero = World->SpawnActor<ARTSHeroCharacter>(HeroClass, SpawnLoc, SpawnRot, SpawnParams);
	if (!NewHero) return nullptr;

	NewHero->HeroId = HeroId;
	NewHero->FactionId = Faction;
	if (!NewHero->InitializeFromRegistry())
	{
		NewHero->Destroy();
		return nullptr;
	}

	// Add to faction squad so the Hero is tracked (optional; Hero may not be in squad in your design).
	UGameInstance* GI = World->GetGameInstance();
	if (GI)
	{
		if (URTSSquadManagerSubsystem* SquadMgr = GI->GetSubsystem<URTSSquadManagerSubsystem>())
		{
			SquadMgr->AddUnitToFactionSquad(NewHero);
		}
	}

	return NewHero;
}
