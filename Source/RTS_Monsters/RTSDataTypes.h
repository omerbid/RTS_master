#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "RTSDataTypes.generated.h"

// MVP-friendly prototypes to bootstrap a data-driven RTS.

UENUM(BlueprintType)
enum class EFactionId : uint8
{
	Humans      UMETA(DisplayName = "Humans"),
	Vampires    UMETA(DisplayName = "Vampires"),
	Werewolves  UMETA(DisplayName = "Werewolves")
};

/** Region type from MASTER_GDD (Map Structure). Used for Phase 2+ settlement/region logic. */
UENUM(BlueprintType)
enum class ERegionType : uint8
{
	Unknown     UMETA(DisplayName = "Unknown"),
	Capital     UMETA(DisplayName = "Capital"),
	City        UMETA(DisplayName = "City"),
	Town        UMETA(DisplayName = "Town"),
	Village     UMETA(DisplayName = "Village"),
	Hamlet      UMETA(DisplayName = "Hamlet"),
	WildForest  UMETA(DisplayName = "Wild Forest"),
	Ruins       UMETA(DisplayName = "Ruins")
};

UENUM(BlueprintType)
enum class EUnitRole : uint8
{
	MeleeLight      UMETA(DisplayName = "Melee Light"),
	MeleeMedium     UMETA(DisplayName = "Melee Medium"),
	RangedLight     UMETA(DisplayName = "Ranged Light"),
	CavalryHeavy    UMETA(DisplayName = "Cavalry Heavy"),
	Assassin        UMETA(DisplayName = "Assassin"),
	Tank            UMETA(DisplayName = "Tank"),
	MeleeSkirmisher UMETA(DisplayName = "Melee Skirmisher"),
	Bruiser         UMETA(DisplayName = "Bruiser"),
	Support         UMETA(DisplayName = "Support")
};

UENUM(BlueprintType)
enum class ESpecialTag : uint8
{
	None           UMETA(DisplayName = "None"),
	ShieldWall     UMETA(DisplayName = "Shield Wall"),
	Ranged         UMETA(DisplayName = "Ranged"),
	Charge         UMETA(DisplayName = "Charge"),
	LifeLeech      UMETA(DisplayName = "Life Leech"),
	StealthBurst   UMETA(DisplayName = "Stealth Burst"),
	FearAura       UMETA(DisplayName = "Fear Aura"),
	PackBonus      UMETA(DisplayName = "Pack Bonus"),
	BerserkTrigger UMETA(DisplayName = "Berserk Trigger"),
	SpiritHowl     UMETA(DisplayName = "Spirit Howl")
};

/** When combat order ends or unit wins a fight: advance, hold ground, or retreat. Per unit type (FUnitRow). */
UENUM(BlueprintType)
enum class ERTSPostCombatBehavior : uint8
{
	Advance UMETA(DisplayName = "Advance"),
	Hold    UMETA(DisplayName = "Hold"),
	Retreat UMETA(DisplayName = "Retreat")
};

/** Current order type. None = no order (idle / post-combat behavior when applicable). */
UENUM(BlueprintType)
enum class ERTSOrderType : uint8
{
	None   UMETA(DisplayName = "None"),
	Move   UMETA(DisplayName = "Move"),
	Attack UMETA(DisplayName = "Attack")
};

/** Phase 3: Human NPC state (CORE_GAME_MECHANICS, PROJECT_BRAIN). */
UENUM(BlueprintType)
enum class EHumanNPCState : uint8
{
	Idle     UMETA(DisplayName = "Idle"),
	Working  UMETA(DisplayName = "Working"),
	Fleeing  UMETA(DisplayName = "Fleeing"),
	Captured UMETA(DisplayName = "Captured")
};

/** Payload for Move (destination) or Attack (target). */
USTRUCT(BlueprintType)
struct FRTSOrderPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector MoveDestination = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<AActor> AttackTarget;
};

USTRUCT(BlueprintType)
struct FUnitRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EFactionId Faction = EFactionId::Humans;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName UnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EUnitRole Role = EUnitRole::MeleeLight;

	// Power scale: Human = 1, Vampire = 2, Werewolf = 3
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 PowerScale = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HP = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Damage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Range = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AttackSpeed = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Armor = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MoveSpeed = 420.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MoraleBase = 60.f;

	/** Upkeep per turn in faction resource: money (Humans), blood (Vampires), flesh (Werewolves). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Upkeep = 1;

	/** Humans: cost in money to recruit. 0 = not recruitable by Humans. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RecruitCostMoney = 50;

	/** Cost in NPC/population to recruit: Humans (e.g. militia), Monsters (conversion). 0 = not using population. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ConvertPopulationCost = 20;

	/** Monsters: cost in faction resource to recruit — blood (Vampires), flesh (Werewolves). 0 = Humans or not recruitable by resource. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RecruitCostResource = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ESpecialTag Special = ESpecialTag::None;

	/** When combat order ends or unit wins: advance, hold, or retreat. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ERTSPostCombatBehavior PostCombatBehavior = ERTSPostCombatBehavior::Hold;

	/** Rank 1–3; Rank ≥ 3 eligible for Captain promotion (P3). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Rank = 1;

	/** Unit level for morale contribution band (Level 1: 0.75–1.0, Level 2: 0.85–1.15, etc.). Default 1. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Level = 1;

	/** Combat target selection: preferred target roles (e.g. RangedLight, Assassin). Empty = no preference. COMBAT_CONTRACT. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "Preferred Target Roles"))
	TArray<FName> PreferredTargetRoles;

	/** Bias for target priority (e.g. focus high-value). 0 = neutral. Future scoring. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "-1", ClampMax = "1"))
	float TargetPriorityBias = 0.f;

	/** Focus-fire factor (0–1): reduce overkill when multiple units same target. 0 = no focus. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0", ClampMax = "1"))
	float FocusFireFactor = 0.f;
};

USTRUCT(BlueprintType)
struct FHeroRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EFactionId Faction = EFactionId::Humans;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName HeroId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Archetype = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 PowerScale = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BaseHP = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BaseDamage = 35.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MoveSpeed = 480.f;

	// Simple MVP aura
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MoraleAura = 0.f;

	/** Command radius in UU. If > 0, overrides component default (2500). Data-driven (GAP_ANALYSIS). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	float CommandRadius = 0.f;

	// XP curve table identifier
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName XPTableId = TEXT("HeroXP_Prototype");

	// Tiered unlocks (store as names for MVP)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Ability_L1 = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Ability_L6 = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Ability_L11 = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Ability_L16 = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Ability_L21 = NAME_None;
};

USTRUCT(BlueprintType)
struct FHeroXPRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName TableId = TEXT("HeroXP_Prototype");

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 XPRequired = 0;
};

USTRUCT(BlueprintType)
struct FMoraleThresholdRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Name = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Min = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Max = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Effect;
};

USTRUCT(BlueprintType)
struct FPredatorPreyStateRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName State = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Effect;
};

