#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RTSCombatTypes.h"
#include "RTSCombatManagerSubsystem.generated.h"

class ARTSUnitCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombatStartedSignature, int32, CombatId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombatRoundResolvedSignature, int32, CombatId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombatEndedSignature, int32, CombatId);

UCLASS()
class RTS_MONSTERS_API URTSCombatManagerSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return true; }

	UFUNCTION(BlueprintCallable, Category = "RTS|Combat")
	int32 StartCombat(const TArray<ARTSUnitCharacter*>& SideAUnits,
		const TArray<ARTSUnitCharacter*>& SideBUnits,
		const FVector& CombatCenter);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RTS|Combat")
	bool IsUnitInCombat(const ARTSUnitCharacter* Unit) const;

	UFUNCTION(BlueprintCallable, Category = "RTS|Combat")
	void ForceEndCombat(int32 CombatId);

	UPROPERTY(BlueprintAssignable)
	FCombatStartedSignature OnCombatStarted;

	UPROPERTY(BlueprintAssignable)
	FCombatRoundResolvedSignature OnCombatRoundResolved;

	UPROPERTY(BlueprintAssignable)
	FCombatEndedSignature OnCombatEnded;

private:
	UPROPERTY()
	TArray<FCombatInstance> ActiveCombats;

	TMap<TWeakObjectPtr<ARTSUnitCharacter>, int32> UnitToCombatId;

	int32 NextCombatId = 1;

	void UpdateCombat(FCombatInstance& Combat, float DeltaTime);
	void EndCombat(FCombatInstance& Combat);
	void RegisterUnitsToCombat(const TArray<ARTSUnitCharacter*>& Units, int32 CombatId);
	void UnregisterUnitsFromCombat(FCombatInstance& Combat);
};
