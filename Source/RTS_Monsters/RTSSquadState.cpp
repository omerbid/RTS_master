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

void URTSSquadState::RecalcMorale()
{
	float Sum = 0.f;
	int32 Count = 0;

	for (const TWeakObjectPtr<ARTSUnitCharacter>& WeakUnit : Members)
	{
		ARTSUnitCharacter* Unit = WeakUnit.Get();
		if (!Unit)
		{
			continue;
		}

		if (const URTSMoraleComponent* MoraleComp = Unit->FindComponentByClass<URTSMoraleComponent>())
		{
			Sum += MoraleComp->CurrentMorale;
			++Count;
		}
	}

	AverageMorale = (Count > 0) ? Sum / static_cast<float>(Count) : 0.f;
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

