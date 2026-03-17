#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RTSOrderManagerSubsystem.generated.h"

/**
 * Centralized order/combat manager. Ticks and updates all units with active orders.
 * Replaces per-unit OrderComponent tick so spawned units (and all units) are updated from one place.
 */
UCLASS()
class RTS_MONSTERS_API URTSOrderManagerSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

private:
	void UpdateAllUnitsWithOrders(float DeltaTime);
};
