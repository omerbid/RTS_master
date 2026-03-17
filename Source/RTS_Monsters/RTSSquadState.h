#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "RTSDataTypes.h"
#include "RTSSquadState.generated.h"

class ARTSUnitCharacter;

/**
 * Runtime representation of a squad (logical grouping of units).
 * Not an Actor – owned by a manager (e.g. GameState or subsystem).
 */
UCLASS(BlueprintType)
class RTS_MONSTERS_API URTSSquadState : public UObject
{
	GENERATED_BODY()

public:
	/** Optional identifier for debugging or UI. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad")
	FName SquadId;

	/** Faction this squad belongs to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad")
	EFactionId FactionId;

	/** Squad morale rating: weighted average by Rank (and Level when available). Rank 1 contributes less, Rank 3+ more; then buffs/debuffs apply. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad")
	float AverageMorale = 0.f;

	/** Get current valid squad members (for Blueprint; TWeakObjectPtr is not Blueprint-reflected). */
	UFUNCTION(BlueprintCallable, Category = "Squad")
	TArray<ARTSUnitCharacter*> GetMembers() const;

	/** Add a unit to the squad. */
	void AddMember(ARTSUnitCharacter* Unit);

	/** Remove a unit from the squad. */
	void RemoveMember(ARTSUnitCharacter* Unit);

	/** Recalculate average morale from member morale components. */
	void RecalcMorale();

	/** P3: Apply morale delta to all current members (e.g. casualty -5, Captain death -10). */
	void ApplyMoraleDeltaToAll(float Delta);

	/** P3: Captain of this squad (optional). Set when a member is promoted. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad")
	TWeakObjectPtr<ARTSUnitCharacter> CaptainUnit;

	/** Set Captain (called when a member is promoted). Clears if Unit is null. */
	void SetCaptain(ARTSUnitCharacter* Unit);

private:
	/** Members stored as weak ptrs so we don't keep destroyed units. Not exposed to Blueprint. */
	TArray<TWeakObjectPtr<ARTSUnitCharacter>> Members;
};

