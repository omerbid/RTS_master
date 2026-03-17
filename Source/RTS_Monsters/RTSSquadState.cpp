#include "RTSSquadState.h"

#include "RTSMoraleComponent.h"
#include "RTSUnitCharacter.h"

void URTSSquadState::AddMember(ARTSUnitCharacter* Unit)
{
	if (!Unit)
	{
		return;
	}

	Members.AddUnique(TWeakObjectPtr<ARTSUnitCharacter>(Unit));
	Unit->SetSquad(this);
	if (Unit->bIsCaptain)
	{
		CaptainUnit = Unit;
	}
	RecalcMorale();
}

void URTSSquadState::RemoveMember(ARTSUnitCharacter* Unit)
{
	if (!Unit)
	{
		return;
	}

	if (CaptainUnit.Get() == Unit)
	{
		CaptainUnit = nullptr;
	}
	Unit->SetSquad(nullptr);
	Members.Remove(TWeakObjectPtr<ARTSUnitCharacter>(Unit));
	RecalcMorale();
}

TArray<ARTSUnitCharacter*> URTSSquadState::GetMembers() const
{
	TArray<ARTSUnitCharacter*> Out;
	for (const TWeakObjectPtr<ARTSUnitCharacter>& WeakUnit : Members)
	{
		if (ARTSUnitCharacter* Unit = WeakUnit.Get())
		{
			Out.Add(Unit);
		}
	}
	return Out;
}

// Squad morale contribution weight by Level and Rank. Level 1 R1–R5 → 0.75–1.0; Level 2 → 0.85–1.15; Level 3+ → 0.95–1.3.
static float GetMoraleContributionWeight(int32 Level, int32 Rank)
{
	const int32 L = FMath::Max(1, Level);
	const int32 R = FMath::Clamp(Rank, 1, 5);
	float MinW, Range;
	if (L == 1)
	{
		MinW = 0.75f;
		Range = 0.25f;   // 0.75 .. 1.0
	}
	else if (L == 2)
	{
		MinW = 0.85f;
		Range = 0.3f;    // 0.85 .. 1.15
	}
	else
	{
		MinW = 0.95f;
		Range = 0.35f;   // 0.95 .. 1.3 for Level 3+
	}
	return MinW + (R - 1) * (Range / 4.f);
}

void URTSSquadState::RecalcMorale()
{
	float WeightedSum = 0.f;
	float TotalWeight = 0.f;

	for (const TWeakObjectPtr<ARTSUnitCharacter>& WeakUnit : Members)
	{
		ARTSUnitCharacter* Unit = WeakUnit.Get();
		if (!Unit)
		{
			continue;
		}

		const URTSMoraleComponent* MoraleComp = Unit->FindComponentByClass<URTSMoraleComponent>();
		if (!MoraleComp)
		{
			continue;
		}

		const float W = GetMoraleContributionWeight(Unit->Level, Unit->Rank);
		WeightedSum += MoraleComp->CurrentMorale * W;
		TotalWeight += W;
	}

	// Weighted average: rating = sum(morale_i * weight_i) / sum(weight_i). Buffs/debuffs apply on top of this base.
	AverageMorale = (TotalWeight > 0.f) ? (WeightedSum / TotalWeight) : 0.f;
}

void URTSSquadState::ApplyMoraleDeltaToAll(float Delta)
{
	for (const TWeakObjectPtr<ARTSUnitCharacter>& WeakUnit : Members)
	{
		if (ARTSUnitCharacter* Unit = WeakUnit.Get())
		{
			if (URTSMoraleComponent* MC = Unit->FindComponentByClass<URTSMoraleComponent>())
			{
				MC->ApplyMoraleDelta(Delta);
			}
		}
	}
	RecalcMorale();
}

void URTSSquadState::SetCaptain(ARTSUnitCharacter* Unit)
{
	CaptainUnit = Unit;
}

