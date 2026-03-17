#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RTSDayNightSubsystem.generated.h"

/** P6: Day/Night phase. GDD: Night grants vampires bonus; day slight penalty. */
UENUM(BlueprintType)
enum class ERTSDayNightPhase : uint8
{
	Day  UMETA(DisplayName = "Day"),
	Night UMETA(DisplayName = "Night")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRTSDayNightPhaseChanged, ERTSDayNightPhase, NewPhase);

/**
 * P6: Day/Night cycle. Single source of truth for IsNight/IsDay.
 * Timer advances phase every PhaseDurationSeconds (e.g. 5 min per phase).
 */
UCLASS()
class RTS_MONSTERS_API URTSDayNightSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RTS|DayNight")
	bool IsNight() const { return bIsNight; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RTS|DayNight")
	bool IsDay() const { return !bIsNight; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RTS|DayNight")
	ERTSDayNightPhase GetCurrentPhase() const { return bIsNight ? ERTSDayNightPhase::Night : ERTSDayNightPhase::Day; }

	/** Seconds until next phase (0 to PhaseDurationSeconds). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RTS|DayNight")
	float GetSecondsUntilNextPhase() const;

	UPROPERTY(BlueprintAssignable, Category = "RTS|DayNight")
	FOnRTSDayNightPhaseChanged OnDayNightPhaseChanged;

	/** Duration of each phase in seconds (60 = 1 min day, 1 min night). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|DayNight", meta = (ClampMin = "1"))
	float PhaseDurationSeconds = 60.f;

	/** Start game in night (false = start in day). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|DayNight")
	bool bStartAtNight = false;

private:
	void TickPhase(float DeltaTime);

	bool bIsNight = false;
	float ElapsedInPhase = 0.f;
	FTimerHandle TimerHandlePhase;
};
