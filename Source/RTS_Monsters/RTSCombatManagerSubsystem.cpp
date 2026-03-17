#include "RTSCombatManagerSubsystem.h"

#include "RTSCombatResolverLibrary.h"
#include "RTSUnitCharacter.h"
#include "RTSHeroCharacter.h"
#include "RTSSaveSubsystem.h"
#include "Engine/World.h"
#include "Stats/Stats.h"

TStatId URTSCombatManagerSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URTSCombatManagerSubsystem, STATGROUP_Tickables);
}

void URTSCombatManagerSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Snapshot count before the loop: StartCombat calls (from delegates) may append new entries;
	// those entries are intentionally skipped this frame to avoid iterator invalidation.
	const int32 NumThisFrame = ActiveCombats.Num();
	for (int32 i = 0; i < NumThisFrame; ++i)
	{
		if (!ActiveCombats.IsValidIndex(i)) break;
		if (ActiveCombats[i].bFinished) continue;
		UpdateCombat(ActiveCombats[i], DeltaTime);
	}

	ActiveCombats.RemoveAll([](const FCombatInstance& Combat) { return Combat.bFinished; });

	// Sweep stale TWeakObjectPtr keys (units destroyed outside EndCombat – edge cases).
	for (auto It = UnitToCombatId.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid()) It.RemoveCurrent();
	}
}

int32 URTSCombatManagerSubsystem::StartCombat(const TArray<ARTSUnitCharacter*>& SideAUnits,
	const TArray<ARTSUnitCharacter*>& SideBUnits,
	const FVector& CombatCenter)
{
	TArray<ARTSUnitCharacter*> ValidA;
	TArray<ARTSUnitCharacter*> ValidB;
	for (ARTSUnitCharacter* U : SideAUnits)
	{
		if (U && IsValid(U) && !IsUnitInCombat(U)) ValidA.Add(U);
	}
	for (ARTSUnitCharacter* U : SideBUnits)
	{
		if (U && IsValid(U) && !IsUnitInCombat(U)) ValidB.Add(U);
	}

	if (ValidA.Num() == 0 || ValidB.Num() == 0)
	{
		return INDEX_NONE;
	}

	FCombatInstance Combat;
	Combat.CombatId = NextCombatId++;
	Combat.CombatCenter = CombatCenter;
	Combat.SideA.DebugName = TEXT("SideA");
	Combat.SideB.DebugName = TEXT("SideB");
	Combat.SideA.Groups.Add(URTSCombatResolverLibrary::BuildGroupFromUnits(ValidA));
	Combat.SideB.Groups.Add(URTSCombatResolverLibrary::BuildGroupFromUnits(ValidB));

	// Set Hero data if present
	for (ARTSUnitCharacter* U : ValidA)
	{
		if (ARTSHeroCharacter* Hero = Cast<ARTSHeroCharacter>(U))
		{
			Combat.SideA.HeroData.bPresent = true;
			Combat.SideA.HeroData.HeroId = Hero->HeroId;
			Combat.SideA.HeroData.LeadershipBonus = 1.1f;
			Combat.SideA.HeroData.MoraleBonus = 5.f;
			break;
		}
	}
	for (ARTSUnitCharacter* U : ValidB)
	{
		if (ARTSHeroCharacter* Hero = Cast<ARTSHeroCharacter>(U))
		{
			Combat.SideB.HeroData.bPresent = true;
			Combat.SideB.HeroData.HeroId = Hero->HeroId;
			Combat.SideB.HeroData.LeadershipBonus = 1.1f;
			Combat.SideB.HeroData.MoraleBonus = 5.f;
			break;
		}
	}

	RegisterUnitsToCombat(ValidA, Combat.CombatId);
	RegisterUnitsToCombat(ValidB, Combat.CombatId);

	UE_LOG(LogTemp, Log, TEXT("[RTS|Combat] StartCombat id=%d SideA=%d units SideB=%d units @ %s"),
		Combat.CombatId, ValidA.Num(), ValidB.Num(), *CombatCenter.ToString());

	ActiveCombats.Add(Combat);
	OnCombatStarted.Broadcast(Combat.CombatId);
	return Combat.CombatId;
}

bool URTSCombatManagerSubsystem::IsUnitInCombat(const ARTSUnitCharacter* Unit) const
{
	if (!Unit) return false;
	// Use non-const pointer for map lookup (key type is TWeakObjectPtr<ARTSUnitCharacter>).
	ARTSUnitCharacter* MutableUnit = const_cast<ARTSUnitCharacter*>(Unit);
	return UnitToCombatId.Contains(TWeakObjectPtr<ARTSUnitCharacter>(MutableUnit));
}

void URTSCombatManagerSubsystem::ForceEndCombat(int32 CombatId)
{
	for (FCombatInstance& Combat : ActiveCombats)
	{
		if (Combat.CombatId == CombatId && !Combat.bFinished)
		{
			EndCombat(Combat);
			return;
		}
	}
}

void URTSCombatManagerSubsystem::UpdateCombat(FCombatInstance& Combat, float DeltaTime)
{
	// SaveLock contract: do not resolve combat during Save snapshot boundary (COMBAT_CONTRACT).
	if (UWorld* W = GetWorld())
	{
		if (UGameInstance* GI = W->GetGameInstance())
		{
			if (URTSSaveSubsystem* SaveSub = GI->GetSubsystem<URTSSaveSubsystem>())
			{
				if (SaveSub->IsSaveLocked())
				{
					return;
				}
			}
		}
	}

	Combat.TimeAccumulator += DeltaTime;
	Combat.ElapsedTime += DeltaTime;

	if (Combat.TimeAccumulator < Combat.ResolveInterval)
	{
		return;
	}

	Combat.TimeAccumulator = 0.f;

	const int32 Seed = HashCombine(GetTypeHash(Combat.CombatId), GetTypeHash(FMath::FloorToInt(Combat.ElapsedTime * 100.f)));
	const FCombatRoundResult RoundResult = URTSCombatResolverLibrary::ResolveCombatRound(Combat, Seed);

	UE_LOG(LogTemp, Verbose, TEXT("[RTS|Combat] CombatId=%d Round: killsToA=%d killsToB=%d SideARouted=%d SideBRouted=%d"),
		Combat.CombatId, RoundResult.EstimatedKillsToSideA, RoundResult.EstimatedKillsToSideB,
		(int32)RoundResult.bSideARouted, (int32)RoundResult.bSideBRouted);

	OnCombatRoundResolved.Broadcast(Combat.CombatId);

	if (RoundResult.bSideARouted || RoundResult.bSideBRouted)
	{
		EndCombat(Combat);
		return;
	}

	bool bSideADead = true;
	for (const FCombatGroup& G : Combat.SideA.Groups)
	{
		if (G.LivingCount > 0) { bSideADead = false; break; }
	}
	bool bSideBDead = true;
	for (const FCombatGroup& G : Combat.SideB.Groups)
	{
		if (G.LivingCount > 0) { bSideBDead = false; break; }
	}

	if (bSideADead || bSideBDead)
	{
		EndCombat(Combat);
	}
}

void URTSCombatManagerSubsystem::EndCombat(FCombatInstance& Combat)
{
	if (Combat.bFinished) return;

	// Engagement lifecycle: notify involved units so PostCombatBehavior fires (Hold/Advance/Retreat).
	auto ApplyPostCombat = [](const FCombatGroup& Group)
	{
		for (const FCombatUnitHandle& Handle : Group.Members)
		{
			if (ARTSUnitCharacter* Unit = Handle.Unit.Get())
			{
				if (IsValid(Unit) && Unit->OrderComponent)
				{
					Unit->OrderComponent->ClearOrder(true);  // true = apply PostCombatBehavior
				}
			}
		}
	};

	for (const FCombatGroup& G : Combat.SideA.Groups) ApplyPostCombat(G);
	for (const FCombatGroup& G : Combat.SideB.Groups) ApplyPostCombat(G);

	UE_LOG(LogTemp, Log, TEXT("[RTS|Combat] CombatId=%d ended. SideA=%d living, SideB=%d living."),
		Combat.CombatId,
		[&]{ int32 N=0; for(auto& G:Combat.SideA.Groups) N+=G.LivingCount; return N; }(),
		[&]{ int32 N=0; for(auto& G:Combat.SideB.Groups) N+=G.LivingCount; return N; }());

	UnregisterUnitsFromCombat(Combat);
	Combat.bFinished = true;
	Combat.SideA.State = ECombatState::Finished;
	Combat.SideB.State = ECombatState::Finished;

	OnCombatEnded.Broadcast(Combat.CombatId);
}

void URTSCombatManagerSubsystem::RegisterUnitsToCombat(const TArray<ARTSUnitCharacter*>& Units, int32 CombatId)
{
	for (ARTSUnitCharacter* U : Units)
	{
		if (U) UnitToCombatId.Add(U, CombatId);
	}
}

void URTSCombatManagerSubsystem::UnregisterUnitsFromCombat(FCombatInstance& Combat)
{
	// Use the TWeakObjectPtr key directly – works even for already-destroyed units
	// (dead units return nullptr from .Get() but their raw pointer key is still in the map).
	auto ClearGroup = [this](const FCombatGroup& Group)
	{
		for (const FCombatUnitHandle& Handle : Group.Members)
		{
			UnitToCombatId.Remove(Handle.Unit);
		}
	};

	for (const FCombatGroup& Group : Combat.SideA.Groups) ClearGroup(Group);
	for (const FCombatGroup& Group : Combat.SideB.Groups) ClearGroup(Group);
}
