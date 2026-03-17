#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RTSDataTypes.h"
#include "RTSHumanNPC.generated.h"

class ARTSRegionVolume;

/**
 * Phase 3: Physical human NPC on the map.
 * CORE_GAME_MECHANICS: "Humans are physical NPCs in the world."
 * States: Idle, Working, Fleeing, Captured.
 * Vampires/Werewolves capture these and convert to units (CaptureHuman).
 */
UCLASS(Blueprintable, meta = (DisplayName = "RTS Human NPC"))
class RTS_MONSTERS_API ARTSHumanNPC : public ACharacter
{
	GENERATED_BODY()

public:
	ARTSHumanNPC();

	/** Stable instance ID for Save/Load (rehydration). Generated in BeginPlay if invalid. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RTS|SaveLoad")
	FGuid PersistentNpcGuid;

	/** For Save/Load: returns PersistentNpcGuid. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RTS|SaveLoad")
	FGuid GetPersistentNpcGuid() const { return PersistentNpcGuid; }
	void SetPersistentNpcGuid(FGuid Id) { PersistentNpcGuid = Id; }

	/** Current state (Idle, Working, Fleeing, Captured). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|NPC")
	EHumanNPCState NPCState = EHumanNPCState::Idle;

	/** Region this NPC belongs to (for population tracking). Set when spawned or placed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|NPC")
	TWeakObjectPtr<ARTSRegionVolume> HomeRegion;

	/** Set state. Captured = cannot move, about to be converted. */
	UFUNCTION(BlueprintCallable, Category = "RTS|NPC")
	void SetNPCState(EHumanNPCState NewState);

	/** Called when a monster (Vampire/Werewolf) captures this NPC. Returns true if captured; NPC will be destroyed after conversion. */
	UFUNCTION(BlueprintCallable, Category = "RTS|NPC")
	bool TryCapture();

	/** True if this NPC can be captured (not already Captured, not Fleeing in safe zone, etc.). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RTS|NPC")
	bool CanBeCaptured() const;

	/** Distance within which a unit can capture this NPC (UU). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|NPC", meta = (ClampMin = "50"))
	float CaptureRange = 300.f;

protected:
	virtual void BeginPlay() override;

	/** If in Captured state, disable movement. */
	void UpdateStateFromCurrent();
};
