#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RTSDataTypes.h"
#include "RTSRegionVolume.generated.h"

class UBoxComponent;
class ARTSHeroCharacter;

/**
 * Region actor representing an area on the map with control levels and stability.
 * GDD: Control 0-5; passive gain to 4 when Hero+units present uncontested; level 5 only via Secure.
 */
UCLASS()
class RTS_MONSTERS_API ARTSRegionVolume : public AActor
{
	GENERATED_BODY()

public:
	ARTSRegionVolume();

	/** Returns true if WorldLocation is inside the region bounds. */
	UFUNCTION(BlueprintCallable, Category = "Region")
	bool IsPointInRegion(FVector WorldLocation) const;

	/** Sets control level for Faction (clamped 0-5) and updates DominantFaction. Level 5 triggers win via VictorySubsystem. */
	UFUNCTION(BlueprintCallable, Category = "Region")
	void SetControlLevelForFaction(EFactionId Faction, int32 Level);

	/** Recomputes DominantFaction from highest control level. */
	UFUNCTION(BlueprintCallable, Category = "Region")
	void RecalcDominantFaction();

	/** Returns the current control level for the given faction. */
	UFUNCTION(BlueprintCallable, Category = "Region")
	int32 GetControlLevelForFaction(EFactionId Faction) const;

	/** Heroes currently overlapping the region (valid only). */
	UFUNCTION(BlueprintCallable, Category = "Region")
	TArray<ARTSHeroCharacter*> GetHeroesInRegion() const;

	/** True if at least one Hero of Faction is inside the region. */
	UFUNCTION(BlueprintCallable, Category = "Region")
	bool HasHeroOfFaction(EFactionId Faction) const;

	/** True if two or more factions have a Hero in region (contested = no control gain). */
	UFUNCTION(BlueprintCallable, Category = "Region")
	bool IsContested() const;

	/** Control level display name for UI/debug (0=None .. 5=Dominance). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Region")
	static FText GetControlLevelName(int32 Level);

	/** Returns the first region that contains WorldLocation (MVP: single region). Pass GetPlayerController(0) or any Actor as WorldContextObject from Blueprint (do not use auto World Context – can crash during PIE startup). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Region")
	static ARTSRegionVolume* GetRegionAtLocation(const UObject* WorldContextObject, FVector WorldLocation);

	/** Stable identity for save/load and serialization (GAP_ANALYSIS). Set in editor or level; used to map saved state to Region actor. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Region")
	FName GetRegionId() const { return RegionId; }

	/** Set stable RegionId (e.g. from level or save). */
	UFUNCTION(BlueprintCallable, Category = "Region")
	void SetRegionId(FName InRegionId) { RegionId = InRegionId; }

	/** P4 Economy: Population for recruitment conversion (Vampires/Werewolves). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Region")
	int32 GetPopulation() const { return Population; }

	/** P4 Economy: Set Population (e.g. when converting). */
	UFUNCTION(BlueprintCallable, Category = "Region")
	void SetPopulation(int32 InPopulation) { Population = FMath::Max(0, InPopulation); }

	/** P4 Economy: Stability for Money rate (Humans). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Region")
	float GetStability() const { return Stability; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRegionBoundsBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRegionBoundsEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void EvaluateControlGain();

	/** Stable identity for save/load (GAP_ANALYSIS). Set per region in level; default from actor label if empty. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	FName RegionId;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Region")
	TObjectPtr<UBoxComponent> RegionBounds;

	/** Control level per faction (0–5). For MVP we store explicit values; later can be generalized. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	int32 ControlLevelHumans;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	int32 ControlLevelVampires;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	int32 ControlLevelWerewolves;

	/** Region population and stability. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	int32 Population;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	float Stability;

	/** Placeholders from Docs (MASTER_GDD_MVP, economy_system, campaign_layer_spec). Used by later phases. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region|Docs")
	float FoodCapacity = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region|Docs")
	int32 WildlifeCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region|Docs")
	float ThreatLevel = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region|Docs")
	float GarrisonPower = 0.f;

	/** 0=None, 1=Normal, 2=Collapsed, etc. (economy_system, MASTER_GDD). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region|Docs")
	int32 SettlementState = 0;

	/** Region type from MASTER_GDD (Capital, City, Town, Village, Hamlet, Wild Forest, Ruins). For Phase 2+ settlement logic. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region|Docs")
	ERegionType RegionType = ERegionType::Unknown;

	/** Dominant faction in this region (based on control). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	EFactionId DominantFaction;

	/** Seconds between control gain checks. Control increases 0->4 when faction has hero in region and not contested. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region", meta = (ClampMin = "1.0"))
	float ControlGainIntervalSeconds = 30.f;

	/** Max level reached by passive gain; 5 only via Secure Region action. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region", meta = (ClampMin = "0", ClampMax = "5"))
	int32 ControlGainMaxLevel = 4;

	FTimerHandle ControlGainTimerHandle;

	/** Actors currently overlapping the region (used for heroes/units presence). Not replicated. */
	TSet<TWeakObjectPtr<AActor>> ActorsInRegion;
};

