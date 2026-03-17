#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RTSDataTypes.h"
#include "RTSEconomySubsystem.generated.h"

class ARTSRegionVolume;
class ARTSHeroCharacter;
class ARTSUnitCharacter;

/**
 * P4: Economy subsystem. Humans gain Money from regions; Vampires/Werewolves convert Population.
 * Money gain: Rate = BaseRate + Population * 0.1 + Stability * 2 + ControlLevel * 5 (per tick).
 */
UCLASS(config = Engine)
class RTS_MONSTERS_API URTSEconomySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Add Money for faction (Humans only; Vampires/Werewolves use Population). */
	UFUNCTION(BlueprintCallable, Category = "RTS|Economy")
	void AddMoney(EFactionId Faction, int32 Amount);

	/** Spend Money. Returns false if insufficient. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Economy")
	bool SpendMoney(EFactionId Faction, int32 Amount);

	/** Get current Money for faction. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RTS|Economy")
	int32 GetMoney(EFactionId Faction) const;

	/** Can faction afford to recruit this unit? Humans: Money >= RecruitCostMoney; Vampires/Werewolves: Region.Population >= ConvertPopulationCost. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RTS|Economy")
	bool CanAffordRecruit(EFactionId Faction, FName UnitId, ARTSRegionVolume* Region) const;

	/** Try to recruit unit. Returns spawned unit or nullptr on failure. Hero must be in region. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Economy")
	ARTSUnitCharacter* TryRecruitUnit(ARTSHeroCharacter* Hero, FName UnitId);

	/** Blueprint class for recruited units (e.g. BP_RTSUnit). Set in Project Settings -> RTS Economy or DefaultEngine.ini. */
	UPROPERTY(Config, EditAnywhere, Category = "RTS|Units")
	FSoftClassPath DefaultUnitClassPath;

private:
	void TickEconomy();

	UPROPERTY()
	TMap<EFactionId, int32> MoneyByFaction;

	FTimerHandle EconomyTickHandle;

	// P4 MVP: hardcoded rates. Rate = BaseRate + Population * PerPop + Stability * PerStab + ControlLevel * PerControl
	static constexpr float MoneyBaseRate = 5.f;
	static constexpr float MoneyPerPopulation = 0.1f;
	static constexpr float MoneyPerStability = 2.f;
	static constexpr float MoneyPerControlLevel = 5.f;
	static constexpr float EconomyTickIntervalSeconds = 5.f;
};
