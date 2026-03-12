#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RTSDataTypes.h"
#include "RTSSecureRegionComponent.generated.h"

class ARTSRegionVolume;
class ARTSHeroCharacter;

/** GDD: Secure Region — available at control 4; Hero in region, not contested; 15s channel; cancel if Hero leaves or enemy Hero enters; success -> level 5. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSecureRegionStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSecureRegionCancelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSecureRegionCompleted, ARTSRegionVolume*, Region);

/**
 * P2 Epic 3: Secure Region action. Attach to Hero. StartSecureRegion() begins a 15s channel;
 * cancelled if Hero leaves region or enemy Hero enters. On success sets region control to 5 (win).
 */
UCLASS(ClassGroup = (RTS), meta = (BlueprintSpawnableComponent))
class RTS_MONSTERS_API URTSSecureRegionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URTSSecureRegionComponent();

	/** GDD: channel duration 15 seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Secure Region", meta = (ClampMin = "0.1"))
	float SecureChannelDurationSeconds = 15.f;

	/** How often to re-validate (Hero still in region, not contested). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Secure Region", meta = (ClampMin = "0.1"))
	float ValidationIntervalSeconds = 0.5f;

	/** Try to start Secure Region. Returns true if channel started. Hero must be in a region with control 4 for own faction, region not contested. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Secure Region")
	bool StartSecureRegion();

	/** Cancel the current Secure channel if active. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Secure Region")
	void CancelSecureRegion();

	/** True while the 15s channel is in progress. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RTS|Secure Region")
	bool IsSecuringRegion() const { return bIsChannelActive; }

	/** Can the owner Hero start Secure Region right now? (in region, control 4, not contested) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RTS|Secure Region")
	bool CanStartSecureRegion() const;

	UPROPERTY(BlueprintAssignable, Category = "RTS|Secure Region")
	FOnSecureRegionStarted OnSecureRegionStarted;

	UPROPERTY(BlueprintAssignable, Category = "RTS|Secure Region")
	FOnSecureRegionCancelled OnSecureRegionCancelled;

	UPROPERTY(BlueprintAssignable, Category = "RTS|Secure Region")
	FOnSecureRegionCompleted OnSecureRegionCompleted;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void ValidateAndTickChannel();
	void CompleteChannel();

	FTimerHandle ChannelTimerHandle;
	FTimerHandle ValidationTimerHandle;
	bool bIsChannelActive = false;
	float ChannelElapsedSeconds = 0.f;

	TWeakObjectPtr<ARTSRegionVolume> SecuringRegion;
};
