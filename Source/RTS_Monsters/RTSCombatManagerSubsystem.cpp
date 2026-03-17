#include "RTSCombatManagerSubsystem.h"

#include "RTSCombatResolverLibrary.h"
#include "RTSUnitCharacter.h"
#include "RTSHeroCharacter.h"
#include "Engine/World.h"
#include "Stats/Stats.h"

TStatId URTSCombatManagerSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URTSCombatManagerSubsystem, STATGROUP_Tickables);
}

void URTSCombatManagerSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (FCombatInstance& Combat : ActiveCombats)
	{
		if (Combat.bFinished) continue;
		UpdateCombat(Combat, DeltaTime);
	}

	ActiveCombats.RemoveAll([](const FCombatInstance& Combat) { return Combat.bFinished; });
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

	ActiveCombats.Add(Combat);
	OnCombatStarted.Broadcast(Combat.CombatId);
	return Combat.CombatId;
}

bool URTSCombatManagerSubsystem::IsUnitInCombat(const ARTSUnitCharacter* Unit) const
{
	if (!Unit) return false;
	return UnitToCombatId.Contains(TWeakObjectPtr<const ARTSUnitCharacter>(const_cast<ARTSUnitCharacter*>(Unit)));
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
	Combat.TimeAccumulator += DeltaTime;
	Combat.ElapsedTime += DeltaTime;

	if (Combat.TimeAccumulator < Combat.ResolveInterval)
	{
		return;
	}

	Combat.TimeAccumulator = 0.f;

	const int32 Seed = HashCombine(GetTypeHash(Combat.CombatId), GetTypeHash(FMath::FloorToInt(Combat.ElapsedTime * 100.f)));
	const FCombatRoundResult RoundResult = URTSCombatResolverLibrary::ResolveCombatRound(Combat, Seed);

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
	auto ClearGroup = [this](const FCombatGroup& Group)
	{
		for (const FCombatUnitHandle& Handle : Group.Members)
		{
			if (ARTSUnitCharacter* U = Handle.Unit.Get())
			{
				UnitToCombatId.Remove(U);
			}
		}
	};

	for (const FCombatGroup& Group : Combat.SideA.Groups) ClearGroup(Group);
	for (const FCombatGroup& Group : Combat.SideB.Groups) ClearGroup(Group);
}
