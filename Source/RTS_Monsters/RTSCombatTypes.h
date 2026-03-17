#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RTSDataTypes.h"
#include "RTSCombatTypes.generated.h"

class ARTSUnitCharacter;

/** Combat system faction (maps to EFactionId). */
UENUM(BlueprintType)
enum class ECombatFaction : uint8
{
	Human    UMETA(DisplayName = "Human"),
	Vampire  UMETA(DisplayName = "Vampire"),
	Werewolf UMETA(DisplayName = "Werewolf"),
	Neutral  UMETA(DisplayName = "Neutral")
};

/** Combat role for damage/armor tables. */
UENUM(BlueprintType)
enum class ECombatRole : uint8
{
	Frontline UMETA(DisplayName = "Frontline"),
	Ranged    UMETA(DisplayName = "Ranged"),
	Monster   UMETA(DisplayName = "Monster"),
	Cavalry   UMETA(DisplayName = "Cavalry"),
	Hero      UMETA(DisplayName = "Hero"),
	Support   UMETA(DisplayName = "Support"),
	Siege     UMETA(DisplayName = "Siege")
};

UENUM(BlueprintType)
enum class EDamageTypeClass : uint8
{
	Blade, Pierce, Claw, Bite, Magic, Siege
};

UENUM(BlueprintType)
enum class EArmorClass : uint8
{
	Light, Medium, Heavy, Monster, Heroic, Structure
};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	None, Active, Retreating, Routed, Finished
};

USTRUCT(BlueprintType)
struct FCombatStatSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECombatFaction Faction = ECombatFaction::Neutral;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECombatRole Role = ECombatRole::Frontline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDamageTypeClass DamageType = EDamageTypeClass::Blade;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EArmorClass ArmorClass = EArmorClass::Medium;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHP = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentHP = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Attack = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Defense = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ArmorPenetration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Morale = 70.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Discipline = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CritChance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Evasion = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LifeSteal = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Fear = 0.f;
};

USTRUCT(BlueprintType)
struct FCombatUnitHandle
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<ARTSUnitCharacter> Unit;

	UPROPERTY()
	FCombatStatSnapshot Snapshot;

	UPROPERTY()
	bool bAlive = true;
};

USTRUCT(BlueprintType)
struct FHeroCombatData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPresent = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName HeroId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LeadershipBonus = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoraleBonus = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AuraAttackBonus = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AuraDefenseBonus = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FearResistanceBonus = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownSeconds = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeUntilAbility = 0.f;
};

USTRUCT(BlueprintType)
struct FCombatGroup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ArchetypeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECombatFaction Faction = ECombatFaction::Neutral;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECombatRole Role = ECombatRole::Frontline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDamageTypeClass DamageType = EDamageTypeClass::Blade;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EArmorClass ArmorClass = EArmorClass::Medium;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCombatUnitHandle> Members;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StartingCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LivingCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalHP = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AvgAttack = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AvgDefense = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AvgAttackSpeed = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AvgRange = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AvgSpeed = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AvgMorale = 70.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AvgDiscipline = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AvgArmorPenetration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AvgLifeSteal = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AvgFear = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FormationBonus = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeroBonus = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TerrainBonus = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FlankBonus = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FatiguePenalty = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRanged = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanRetreat = true;
};

USTRUCT(BlueprintType)
struct FCombatSide
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCombatGroup> Groups;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FHeroCombatData HeroData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SideMorale = 75.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LeadershipBonus = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FearResistance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECombatState State = ECombatState::Active;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DebugName;
};

USTRUCT(BlueprintType)
struct FCombatRoundResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EstimatedKillsToSideA = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EstimatedKillsToSideB = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoraleDeltaSideA = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoraleDeltaSideB = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSideARouted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSideBRouted = false;
};

USTRUCT(BlueprintType)
struct FCombatInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CombatId = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCombatSide SideA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCombatSide SideB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CombatCenter = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ResolveInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeAccumulator = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ElapsedTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFinished = false;
};
