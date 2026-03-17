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

/** P5: Respawn state per faction after Hero death when faction has region >= 3. */
UENUM(BlueprintType)
enum class ERTSRespawnState : uint8
{
	None              UMETA(DisplayName = "None"),
	WaitingForTimer   UMETA(DisplayName = "Waiting For Timer"),
	RitualAvailable   UMETA(DisplayName = "Ritual Available"),
	ChannelingRitual  UMETA(DisplayName = "Channeling Ritual")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRTSGameWon, EFactionId, Faction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRTSGameLost, EFactionId, Faction);
/** P5: Broadcast when respawn timer ends and player can Perform Ritual. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRTSRespawnRitualAvailable, EFactionId, Faction);

/**
 * P2 Win/Lose: notifies when control reaches 5 (win) or faction has no hero and no region >= 3 (lose).
 * P5: Hero respawn ritual – when Hero dies and faction has region >= 3, start 90s timer then ritual (10s channel) -> spawn in best region.
 */
UCLASS()
class RTS_MONSTERS_API URTSVictorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Called when any faction reaches control level 5 in a region. */
	void NotifyControlReachedFive(ARTSRegionVolume* Region, EFactionId Faction);

	/** Called when a Hero dies; either lose (no region >= 3) or start respawn timer (P5). */
	void NotifyHeroDeath(ARTSHeroCharacter* Hero);

	UFUNCTION(BlueprintCallable, Category = "RTS|Victory")
	ERTSGameResult GetGameResult() const { return GameResult; }

	UFUNCTION(BlueprintCallable, Category = "RTS|Victory")
	EFactionId GetWinningFaction() const { return WinningFaction; }

	UFUNCTION(BlueprintCallable, Category = "RTS|Victory")
	EFactionId GetLosingFaction() const { return LosingFaction; }

	UFUNCTION(BlueprintCallable, Category = "RTS|Victory")
	bool IsGameOver() const { return GameResult != ERTSGameResult::Playing; }

	// ---------- P5 Respawn Ritual ----------
	/** GDD: 90s respawn timer (tunable). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Respawn", meta = (ClampMin = "1"))
	float RespawnTimerSeconds = 90.f;

	/** GDD: 10s ritual channel (tunable). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Respawn", meta = (ClampMin = "0.5"))
	float RitualChannelSeconds = 10.f;

	/** Morale shock radius (UU) when Hero dies. Squads with a member in this range get penalty. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Respawn", meta = (ClampMin = "0"))
	float MoraleShockRadius = 2500.f;

	/** Morale delta applied to squads in range when Hero dies (e.g. -15). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Respawn")
	float MoraleShockDelta = -15.f;

	/** Current respawn state for a faction (P5). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RTS|Respawn")
	ERTSRespawnState GetRespawnState(EFactionId Faction) const;

	/** True if player can press Perform Ritual for this faction (timer ended, ritual available). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RTS|Respawn")
	bool CanPerformRitual(EFactionId Faction) const;

	/** If any faction is in RitualAvailable state, returns it (for single-player: the player's faction). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RTS|Respawn")
	bool GetFactionWithRitualAvailable(EFactionId& OutFaction) const;

	/** Start the 10s ritual channel. Returns false if not RitualAvailable or already channeling. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Respawn")
	bool StartRitualChannel(EFactionId Faction);

	/** Cancel the ritual channel if active (e.g. on player move – optional). */
	UFUNCTION(BlueprintCallable, Category = "RTS|Respawn")
	void CancelRitualChannel(EFactionId Faction);

	/** Hero class to spawn for respawn (default: C++ ARTSHeroCharacter). Set in Blueprint to use BP_RTSHero. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Respawn")
	TSubclassOf<ARTSHeroCharacter> HeroRespawnClass;

	UPROPERTY(BlueprintAssignable, Category = "RTS|Victory")
	FOnRTSGameWon OnGameWon;

	UPROPERTY(BlueprintAssignable, Category = "RTS|Victory")
	FOnRTSGameLost OnGameLost;

	UPROPERTY(BlueprintAssignable, Category = "RTS|Respawn")
	FOnRTSRespawnRitualAvailable OnRespawnRitualAvailable;

private:
	void CheckLoseCondition(UWorld* World, EFactionId Faction);
	/** P5: Check if faction has at least one region with control >= 3. */
	bool HasRegionWithControlAtLeast3(UWorld* World, EFactionId Faction) const;
	/** P5: Apply morale shock to squads with any member in range of Location. */
	void ApplyMoraleShockToSquadsNear(UWorld* World, FVector Location, EFactionId Faction);
	/** P5: Start respawn timer for Faction; store HeroId for spawn. */
	void StartRespawnTimer(EFactionId Faction, FName HeroIdToRespawn);
	void OnRespawnTimerExpired(EFactionId Faction);
	void OnRitualChannelComplete(EFactionId Faction);
	void OnRespawnTimerExpired_Humans() { OnRespawnTimerExpired(EFactionId::Humans); }
	void OnRespawnTimerExpired_Vampires() { OnRespawnTimerExpired(EFactionId::Vampires); }
	void OnRespawnTimerExpired_Werewolves() { OnRespawnTimerExpired(EFactionId::Werewolves); }
	void OnRitualChannelComplete_Humans() { OnRitualChannelComplete(EFactionId::Humans); }
	void OnRitualChannelComplete_Vampires() { OnRitualChannelComplete(EFactionId::Vampires); }
	void OnRitualChannelComplete_Werewolves() { OnRitualChannelComplete(EFactionId::Werewolves); }
	/** P5: Find region with highest control for Faction (>= 3), spawn Hero there. */
	ARTSHeroCharacter* SpawnHeroInBestRegion(UWorld* World, EFactionId Faction, FName HeroId);
	static uint8 FactionToIndex(EFactionId Faction);

	UPROPERTY()
	ERTSGameResult GameResult = ERTSGameResult::Playing;

	UPROPERTY()
	EFactionId WinningFaction = EFactionId::Humans;

	UPROPERTY()
	EFactionId LosingFaction = EFactionId::Humans;

	// P5: Respawn state per faction (index by uint8(EFactionId))
	ERTSRespawnState RespawnStateByFaction[3] = { ERTSRespawnState::None, ERTSRespawnState::None, ERTSRespawnState::None };
	FName RespawnHeroIdByFaction[3] = { NAME_None, NAME_None, NAME_None };
	FTimerHandle RespawnTimerHandleByFaction[3];
	FTimerHandle RitualChannelHandleByFaction[3];
};
