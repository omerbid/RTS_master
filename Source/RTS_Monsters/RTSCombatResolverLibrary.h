#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RTSCombatTypes.h"
#include "RTSCombatResolverLibrary.generated.h"

class ARTSUnitCharacter;

UCLASS()
class RTS_MONSTERS_API URTSCombatResolverLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "RTS|Combat")
	static FCombatGroup BuildGroupFromUnits(const TArray<ARTSUnitCharacter*>& Units);

	UFUNCTION(BlueprintPure, Category = "RTS|Combat")
	static float GetDamageTypeVsArmorMultiplier(EDamageTypeClass DamageType, EArmorClass ArmorClass);

	UFUNCTION(BlueprintPure, Category = "RTS|Combat")
	static float ComputeMoraleModifier(float Morale);

	UFUNCTION(BlueprintPure, Category = "RTS|Combat")
	static float ComputeGroupResolvePower(const FCombatGroup& Group, const FCombatSide& OwningSide);

	UFUNCTION(BlueprintPure, Category = "RTS|Combat")
	static float ComputeDefenseMitigation(const FCombatGroup& Defender);

	UFUNCTION(BlueprintPure, Category = "RTS|Combat")
	static int32 ChooseTargetGroupIndex(const FCombatGroup& Attacker, const FCombatSide& EnemySide);

	UFUNCTION(BlueprintCallable, Category = "RTS|Combat")
	static FCombatRoundResult ResolveCombatRound(FCombatInstance& Combat, int32 RandomSeed);

	/** Apply damage to group: reduces each unit's HP gradually, destroys when HP <= 0. Returns kill count. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Combat")
	static int32 ApplyDamageToGroup(FCombatGroup& Group, float TotalDamage);

	UFUNCTION(BlueprintCallable, Category = "RTS|Combat")
	static void RefreshGroupState(FCombatGroup& Group);

	UFUNCTION(BlueprintPure, Category = "RTS|Combat")
	static bool ShouldSideRout(const FCombatSide& Side);

	static ECombatFaction FactionIdToCombatFaction(EFactionId FactionId);
	static ECombatRole UnitRoleToCombatRole(EUnitRole Role);
	static EDamageTypeClass RoleToDamageType(EUnitRole Role);
	static EArmorClass RoleToArmorClass(EUnitRole Role);

private:
	static float GetHeroBonus(const FCombatSide& Side);
	static void ApplyRoundMorale(FCombatSide& Side, int32 CasualtiesTaken, int32 CasualtiesInflicted);
};
