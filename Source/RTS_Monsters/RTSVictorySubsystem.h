#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RTSDataTypes.h"
#include "RTSVictorySubsystem.generated.h"

class ARTSRegionVolume;
class ARTSHeroCharacter;

/** GDD: Win = reach control 5. Lose = no living hero AND no region with control >= 3 for that faction. */
UENUM(BlueprintType)
enum class ERTSGameResult : uint8
{
	Playing UMETA(DisplayName = "Playing"),
	Won    UMETA(DisplayName = "Won"),
	Lost   UMETA(DisplayName = "Lost")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRTSGameWon, EFactionId, Faction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRTSGameLost, EFactionId, Faction);

/**
 * P2 Win/Lose: notifies when control reaches 5 (win) or faction has no hero and no region >= 3 (lose).
 */
UCLASS()
class RTS_MONSTERS_API URTSVictorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Called when any faction reaches control level 5 in a region. */
	void NotifyControlReachedFive(ARTSRegionVolume* Region, EFactionId Faction);

	/** Called when a Hero dies; evaluates lose for that faction (no living hero + no region >= 3). */
	void NotifyHeroDeath(ARTSHeroCharacter* Hero);

	UFUNCTION(BlueprintCallable, Category = "RTS|Victory")
	ERTSGameResult GetGameResult() const { return GameResult; }

	UFUNCTION(BlueprintCallable, Category = "RTS|Victory")
	EFactionId GetWinningFaction() const { return WinningFaction; }

	UFUNCTION(BlueprintCallable, Category = "RTS|Victory")
	EFactionId GetLosingFaction() const { return LosingFaction; }

	UFUNCTION(BlueprintCallable, Category = "RTS|Victory")
	bool IsGameOver() const { return GameResult != ERTSGameResult::Playing; }

	UPROPERTY(BlueprintAssignable, Category = "RTS|Victory")
	FOnRTSGameWon OnGameWon;

	UPROPERTY(BlueprintAssignable, Category = "RTS|Victory")
	FOnRTSGameLost OnGameLost;

private:
	void CheckLoseCondition(UWorld* World, EFactionId Faction);

	UPROPERTY()
	ERTSGameResult GameResult = ERTSGameResult::Playing;

	UPROPERTY()
	EFactionId WinningFaction = EFactionId::Humans;

	UPROPERTY()
	EFactionId LosingFaction = EFactionId::Humans;
};
