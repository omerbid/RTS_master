#include "RTSSquadManagerSubsystem.h"

#include "RTSSquadState.h"
#include "RTSUnitCharacter.h"

URTSSquadState* URTSSquadManagerSubsystem::GetOrCreateSquadForFaction(EFactionId FactionId)
{
	if (TObjectPtr<URTSSquadState>* Found = SquadsByFaction.Find(FactionId))
	{
		return *Found;
	}
	URTSSquadState* Squad = NewObject<URTSSquadState>(this);
	Squad->FactionId = FactionId;
	Squad->SquadId = FName(*FString::Printf(TEXT("Squad_%d"), static_cast<int32>(FactionId)));
	SquadsByFaction.Add(FactionId, Squad);
	return Squad;
}

void URTSSquadManagerSubsystem::AddUnitToFactionSquad(ARTSUnitCharacter* Unit)
{
	if (!Unit)
	{
		return;
	}
	URTSSquadState* Squad = GetOrCreateSquadForFaction(Unit->FactionId);
	if (Squad)
	{
		Squad->AddMember(Unit);
	}
}
