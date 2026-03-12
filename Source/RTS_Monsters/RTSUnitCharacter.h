#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RTSDataTypes.h"
#include "RTSOrderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "RTSUnitCharacter.generated.h"

class URTSCommandAuthorityComponent;
class URTSMoraleComponent;
class URTSSquadState;

/**
 * Base RTS unit character.
 * Stats and faction are data-driven via FUnitRow.
 * Create a Blueprint from this class (e.g. BP_RTSUnit) to place in the level.
 */
UCLASS(Blueprintable, meta = (DisplayName = "RTS Unit Character"))
class RTS_MONSTERS_API ARTSUnitCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ARTSUnitCharacter();

	/** Faction this unit belongs to (Humans / Vampires / Werewolves). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS")
	EFactionId FactionId;

	/** Unit identifier used to look up data table rows. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Data")
	FName UnitId;

	/** Cached unit data row (filled from Registry by UnitId; editable here to override per instance). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Data")
	FUnitRow CachedUnitData;

	/** If > 0, overrides CachedUnitData.MoveSpeed after init from Registry. Lets one instance differ from the table. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Data", meta = (ClampMin = "0"))
	float OverrideMoveSpeed = 0.f;

	/** If > 0, overrides CachedUnitData.HP after init from Registry. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Data", meta = (ClampMin = "0"))
	float OverrideHP = 0.f;

	/** If > 0, overrides CachedUnitData.Damage after init from Registry. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Data", meta = (ClampMin = "0"))
	float OverrideDamage = 0.f;

	/** If false, do not load from DataTable in BeginPlay; use only the values you set in Cached Unit Data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Data")
	bool bInitializeFromRegistry = true;

	/** Morale component for this unit. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RTS|Components")
	URTSMoraleComponent* MoraleComponent;

	/** Order component: current order and execution (Move / Attack). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RTS|Components")
	URTSOrderComponent* OrderComponent;

	/** Squad this unit belongs to (set when added via URTSSquadState::AddMember). P3. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RTS|Squad")
	TWeakObjectPtr<URTSSquadState> SquadReference;

	/** Set by URTSSquadState::AddMember/RemoveMember; prefer using squad API. */
	void SetSquad(URTSSquadState* Squad);

	/** Initialize from a unit data row (called after spawn). */
	void InitializeFromUnitRow(const FUnitRow& UnitRow);

	/** Load UnitId from DataRegistry and initialize. Returns false if registry or row missing. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Data")
	virtual bool InitializeFromRegistry();

	/** Set current order (Move or Attack). Persists until replaced or completed (GDD). */
	UFUNCTION(BlueprintCallable, Category = "RTS|Command")
	void SetCurrentOrder(ERTSOrderType OrderType, const FRTSOrderPayload& Payload);

	/** P3: Rank 1–3 from data; Rank ≥ 3 eligible for Captain. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Captain")
	int32 Rank;

	/** P3: True when unit is promoted to Captain (has CommandAuthority 1200 UU). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Captain")
	bool bIsCaptain;

	/** P3: Command authority when promoted to Captain (1200 UU). Created on promote. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RTS|Components")
	URTSCommandAuthorityComponent* CommandAuthorityComponent;

	/** Promote to Captain if eligible (Rank ≥ 3, same faction as issuer, not Hero). Returns true if promoted. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Captain")
	bool TryPromoteToCaptain(const ARTSUnitCharacter* Issuer);

	/** Update selection visual (ring + custom depth). Called by PlayerController when selection changes. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Selection")
	void SetSelected(bool bSelected);

	/** P3: True when no Hero/Captain of same faction in command range (defensive only, morale drain). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RTS|Morale")
	bool bIsDetached = false;

	/** P3: 0.85 when morale < 30, else 1. Used for move speed and order responsiveness (GDD). */
	UFUNCTION(BlueprintPure, Category = "RTS|Morale")
	float GetOrderResponsivenessMultiplier() const;

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	/** P3: Update MaxWalkSpeed and cached multipliers from morale; run every 1s. */
	void UpdateMoraleEffects();
	/** P3: Check detached (authority in range); run every 2s. Apply drain -1/5s when detached. */
	void UpdateDetachedAndDrain();
	/** P3: When morale < 15, 20% every 5s trigger auto-retreat. */
	void TickLowMoraleAutoRetreat();

	/** Flat ring at feet for selection visual. */
	UPROPERTY(VisibleAnywhere, Category = "RTS|Selection")
	TObjectPtr<UStaticMeshComponent> SelectionRingMesh;

	FTimerHandle TimerHandleMoraleUpdate;
	FTimerHandle TimerHandleDetachedAndDrain;
};

