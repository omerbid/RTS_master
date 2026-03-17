#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RTSDataTypes.h"
#include "RTSDataRegistry.generated.h"

class UDataTable;

/**
 * Central access to RTS data tables. Loads DataTables at startup and provides lookup by ID.
 * GDD-aligned: data-driven units, heroes, morale thresholds, hero XP.
 */
UCLASS()
class RTS_MONSTERS_API URTSDataRegistry : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Get unit row by UnitId. Returns false if table missing or row not found. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Data")
	bool GetUnitRow(FName UnitId, FUnitRow& OutRow) const;

	/** Get hero row by HeroId. Returns false if table missing or row not found. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Data")
	bool GetHeroRow(FName HeroId, FHeroRow& OutRow) const;

	/** Get hero XP row by table id and level. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Data")
	bool GetHeroXPRow(FName TableId, int32 Level, FHeroXPRow& OutRow) const;

	/** Get morale threshold row by name. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Data")
	bool GetMoraleThresholdRow(FName Name, FMoraleThresholdRow& OutRow) const;

	/** Get unit row for a hero (base stats). Uses Hero row's faction and a fallback. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Data")
	bool GetUnitRowForHero(const FHeroRow& HeroRow, FUnitRow& OutUnitRow) const;

	/** P4: Get first recruitable UnitId for faction. Humans: RecruitCostMoney>0; Vampires/Werewolves: ConvertPopulationCost>0. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Data")
	FName GetFirstRecruitableUnitIdForFaction(EFactionId Faction) const;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	/** Loaded at startup from Content/Data/ paths. See .cursor/docs/DATA_TABLES_CHECKLIST.md */
	TObjectPtr<UDataTable> UnitsTable;
	TObjectPtr<UDataTable> HeroesTable;
	TObjectPtr<UDataTable> HeroXPTable;
	TObjectPtr<UDataTable> MoraleThresholdsTable;

	void LoadTablesFromPaths();
	bool ValidateAndLog();
};
