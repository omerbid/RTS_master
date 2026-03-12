#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RTSDataTypes.generated.h"

// MVP-friendly prototypes to bootstrap a data-driven RTS.

UENUM(BlueprintType)
enum class EFactionId : uint8
{
    Humans      UMETA(DisplayName="Humans"),
    Vampires    UMETA(DisplayName="Vampires"),
    Werewolves  UMETA(DisplayName="Werewolves")
};

UENUM(BlueprintType)
enum class EUnitRole : uint8
{
    MeleeLight      UMETA(DisplayName="Melee Light"),
    MeleeMedium     UMETA(DisplayName="Melee Medium"),
    RangedLight     UMETA(DisplayName="Ranged Light"),
    CavalryHeavy    UMETA(DisplayName="Cavalry Heavy"),
    Assassin        UMETA(DisplayName="Assassin"),
    Tank            UMETA(DisplayName="Tank"),
    MeleeSkirmisher UMETA(DisplayName="Melee Skirmisher"),
    Bruiser         UMETA(DisplayName="Bruiser"),
    Support         UMETA(DisplayName="Support")
};

UENUM(BlueprintType)
enum class ESpecialTag : uint8
{
    None            UMETA(DisplayName="None"),
    ShieldWall      UMETA(DisplayName="Shield Wall"),
    Ranged          UMETA(DisplayName="Ranged"),
    Charge          UMETA(DisplayName="Charge"),
    LifeLeech       UMETA(DisplayName="Life Leech"),
    StealthBurst    UMETA(DisplayName="Stealth Burst"),
    FearAura        UMETA(DisplayName="Fear Aura"),
    PackBonus       UMETA(DisplayName="Pack Bonus"),
    BerserkTrigger  UMETA(DisplayName="Berserk Trigger"),
    SpiritHowl      UMETA(DisplayName="Spirit Howl")
};

USTRUCT(BlueprintType)
struct FUnitRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) EFactionId Faction = EFactionId::Humans;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName UnitId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) EUnitRole Role = EUnitRole::MeleeLight;

    // Power scale: Human=1, Vampire=2, Werewolf=3
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 PowerScale = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly) float HP = 100.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float Damage = 10.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float Range = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float AttackSpeed = 1.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float Armor = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float MoveSpeed = 420.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float MoraleBase = 60.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Upkeep = 1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) ESpecialTag Special = ESpecialTag::None;
};

USTRUCT(BlueprintType)
struct FHeroRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) EFactionId Faction = EFactionId::Humans;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName HeroId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Archetype = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 PowerScale = 1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float BaseHP = 600.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float BaseDamage = 35.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float MoveSpeed = 480.f;

    // Simple MVP aura
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float MoraleAura = 0.f;

    // XP curve table identifier
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName XPTableId = TEXT("HeroXP_Prototype");

    // Tiered unlocks (store as names for MVP)
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Ability_L1 = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Ability_L6 = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Ability_L11 = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Ability_L16 = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Ability_L21 = NAME_None;
};

USTRUCT(BlueprintType)
struct FHeroXPRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName TableId = TEXT("HeroXP_Prototype");
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Level = 1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 XPRequired = 0;
};

USTRUCT(BlueprintType)
struct FMoraleThresholdRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Name = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Min = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Max = 100;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText Effect;
};

USTRUCT(BlueprintType)
struct FPredatorPreyStateRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName State = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText Effect;
};
