#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RTSMoraleComponent.generated.h"

/**
 * Per-unit morale component.
 * Source of truth for CurrentMorale; squad-level systems can aggregate these values.
 */
UCLASS(ClassGroup = (RTS), meta = (BlueprintSpawnableComponent))
class RTS_MONSTERS_API URTSMoraleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URTSMoraleComponent();

	/** Current morale value [0,100]. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morale")
	float CurrentMorale;

	/** Base morale for this unit (typically from data table). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morale")
	float BaseMorale;

	/** Minimum and maximum clamp for morale. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morale")
	float MinMorale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morale")
	float MaxMorale;

	/** Apply a delta to morale and return the new value. */
	UFUNCTION(BlueprintCallable, Category = "Morale")
	float ApplyMoraleDelta(float Delta);

	/** Set morale directly, clamped to [MinMorale, MaxMorale]. */
	UFUNCTION(BlueprintCallable, Category = "Morale")
	void SetMorale(float NewValue);

protected:
	virtual void BeginPlay() override;
};

