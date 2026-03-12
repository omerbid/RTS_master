#pragma once

#include "CoreMinimal.h"
#include "RTSUnitCharacter.h"
#include "RTSHeroCharacter.generated.h"

class URTSCommandAuthorityComponent;
class URTSSecureRegionComponent;

/**
 * Base RTS hero character (extends unit with command authority and hero-specific data).
 * Create a Blueprint from this class (e.g. BP_RTSHero) to place in the level.
 */
UCLASS(Blueprintable, meta = (DisplayName = "RTS Hero Character"))
class RTS_MONSTERS_API ARTSHeroCharacter : public ARTSUnitCharacter
{
	GENERATED_BODY()

public:
	ARTSHeroCharacter();

	/** Hero identifier used to look up hero data table rows. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Data")
	FName HeroId;

	/** Cached hero data row (optional). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RTS|Data")
	FHeroRow CachedHeroData;

	/** Command authority inherited from ARTSUnitCharacter (Hero creates it with 2500 UU in ctor). */

	/** P2: Secure Region action (15s channel at control 4; success -> level 5). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RTS|Components")
	URTSSecureRegionComponent* SecureRegionComponent;

	/** Try to start Secure Region. Returns true if channel started. Bind to input (e.g. Hold + click or key). */
	UFUNCTION(BlueprintCallable, Category = "RTS|Secure Region")
	bool TryStartSecureRegion();

	/** Initialize from hero + unit data rows. */
	void InitializeFromHeroRow(const FHeroRow& HeroRow, const FUnitRow& AsUnitRow);

	/** Override: load HeroId from DataRegistry and initialize. Returns false if registry or rows missing. */
	bool InitializeFromRegistry() override;

	/** Example stub for issuing orders – will be expanded in P1. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Command")
	void IssueOrderToUnitsInRange(const TArray<ARTSUnitCharacter*>& Units);

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
};

