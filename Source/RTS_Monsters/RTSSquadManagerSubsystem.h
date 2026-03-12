#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RTSDataTypes.h"
#include "RTSSquadManagerSubsystem.generated.h"

class URTSSquadState;
class ARTSUnitCharacter;

/** P3: Creates and owns squads per faction; assigns units to their faction squad on request. */
UCLASS()
class RTS_MONSTERS_API URTSSquadManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Get or create the squad for this faction. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Squad")
	URTSSquadState* GetOrCreateSquadForFaction(EFactionId FactionId);

	/** Add unit to its faction's squad (call from unit BeginPlay or when spawning). */
	UFUNCTION(BlueprintCallable, Category = "RTS|Squad")
	void AddUnitToFactionSquad(ARTSUnitCharacter* Unit);

private:
	UPROPERTY()
	TMap<EFactionId, TObjectPtr<URTSSquadState>> SquadsByFaction;
};
