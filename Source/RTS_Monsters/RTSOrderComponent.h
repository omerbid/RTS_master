#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RTSDataTypes.h"
#include "RTSOrderComponent.generated.h"

class ARTSUnitCharacter;

/**
 * Holds and executes the unit's current order (Move / Attack).
 * Persistence: order is kept until replaced or completed; no auto-cancel when leaving command range (GDD).
 */
UCLASS(ClassGroup = (RTS), meta = (BlueprintSpawnableComponent))
class RTS_MONSTERS_API URTSOrderComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URTSOrderComponent();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RTS|Order")
	ERTSOrderType CurrentOrderType = ERTSOrderType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RTS|Order")
	FRTSOrderPayload CurrentOrderPayload;

	/** Acceptance radius for Move order (UU). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Order", meta = (ClampMin = "10"))
	float MoveAcceptanceRadius = 80.f;

	/** Set current order (replaces any existing). Move: pass destination in Payload.MoveDestination. Attack: pass target in Payload.AttackTarget. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Order")
	void SetCurrentOrder(ERTSOrderType OrderType, const FRTSOrderPayload& Payload);

	/** Clear current order. Optionally apply post-combat behavior from unit type data. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Order")
	void ClearOrder(bool bApplyPostCombatBehavior = true);

	UFUNCTION(BlueprintPure, Category = "RTS|Order")
	bool HasOrder() const { return CurrentOrderType != ERTSOrderType::None; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

private:
	/** Execute Move: drive character toward MoveDestination. */
	void TickMove(float DeltaTime);
	/** Execute Attack: move toward target; when in range, attack (stub). Combat end triggers post-combat. */
	void TickAttack(float DeltaTime);
	/** Apply post-combat behavior from CachedUnitData (Advance / Hold / Retreat). */
	void ApplyPostCombatBehavior();

	UPROPERTY()
	TWeakObjectPtr<ARTSUnitCharacter> CachedUnit;
};
