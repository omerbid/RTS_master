#include "RTSCombatResolverLibrary.h"

#include "RTSUnitCharacter.h"
#include "RTSHeroCharacter.h"
#include "RTSMoraleComponent.h"
#include "Algo/Sort.h"
#include "Math/RandomStream.h"

static constexpr float GMinMoraleModifier = 0.50f;
static constexpr float GMaxMoraleModifier = 1.35f;
static constexpr float GDefenseWeight = 0.35f;
static constexpr float GArmorPenWeight = 0.20f;
static constexpr float GCasualtyShock = 2.75f;
static constexpr float GKillMoraleReward = 0.75f;

ECombatFaction URTSCombatResolverLibrary::FactionIdToCombatFaction(EFactionId FactionId)
{
	switch (FactionId)
	{
	case EFactionId::Humans: return ECombatFaction::Human;
	case EFactionId::Vampires: return ECombatFaction::Vampire;
	case EFactionId::Werewolves: return ECombatFaction::Werewolf;
	default: return ECombatFaction::Neutral;
	}
}

ECombatRole URTSCombatResolverLibrary::UnitRoleToCombatRole(EUnitRole Role)
{
	switch (Role)
	{
	case EUnitRole::RangedLight: return ECombatRole::Ranged;
	case EUnitRole::CavalryHeavy: return ECombatRole::Cavalry;
	case EUnitRole::Bruiser:
	case EUnitRole::Tank: return ECombatRole::Monster;
	case EUnitRole::Support: return ECombatRole::Support;
	default: return ECombatRole::Frontline;
	}
}

EDamageTypeClass URTSCombatResolverLibrary::RoleToDamageType(EUnitRole Role)
{
	switch (Role)
	{
	case EUnitRole::RangedLight: return EDamageTypeClass::Pierce;
	case EUnitRole::Bruiser:
	case EUnitRole::Assassin: return EDamageTypeClass::Claw;
	default: return EDamageTypeClass::Blade;
	}
}

EArmorClass URTSCombatResolverLibrary::RoleToArmorClass(EUnitRole Role)
{
	switch (Role)
	{
	case EUnitRole::MeleeLight:
	case EUnitRole::RangedLight: return EArmorClass::Light;
	case EUnitRole::CavalryHeavy:
	case EUnitRole::Tank: return EArmorClass::Heavy;
	default: return EArmorClass::Medium;
	}
}

FCombatGroup URTSCombatResolverLibrary::BuildGroupFromUnits(const TArray<ARTSUnitCharacter*>& Units)
{
	FCombatGroup Group;

	for (ARTSUnitCharacter* Unit : Units)
	{
		if (!Unit || !IsValid(Unit))
		{
			continue;
		}

		FCombatUnitHandle Handle;
		Handle.Unit = Unit;

		const FUnitRow& Row = Unit->CachedUnitData;
		Handle.Snapshot.UnitId = Unit->UnitId;
		Handle.Snapshot.Faction = FactionIdToCombatFaction(Unit->FactionId);
		Handle.Snapshot.Role = Unit->bIsCaptain || Cast<ARTSHeroCharacter>(Unit) ? ECombatRole::Hero : UnitRoleToCombatRole(Row.Role);
		Handle.Snapshot.DamageType = RoleToDamageType(Row.Role);
		Handle.Snapshot.ArmorClass = RoleToArmorClass(Row.Role);

		const float HP = Unit->OverrideHP > 0.f ? Unit->OverrideHP : Row.HP;
		const float Dmg = Unit->OverrideDamage > 0.f ? Unit->OverrideDamage : Row.Damage;
		Handle.Snapshot.MaxHP = HP;
		Handle.Snapshot.CurrentHP = HP;
		Handle.Snapshot.Attack = Dmg * Unit->GetDayNightDamageMultiplier();  // P6: Vampire night bonus / day penalty
		Handle.Snapshot.Defense = Row.Armor > 0.f ? Row.Armor : 5.f;
		Handle.Snapshot.AttackSpeed = Row.AttackSpeed > 0.f ? Row.AttackSpeed : 1.f;
		Handle.Snapshot.Range = Row.Range > 0.f ? Row.Range : 150.f;
		Handle.Snapshot.Speed = Row.MoveSpeed > 0.f ? Row.MoveSpeed : 350.f;
		Handle.Snapshot.Morale = Row.MoraleBase > 0.f ? Row.MoraleBase : 70.f;
		if (URTSMoraleComponent* MC = Unit->FindComponentByClass<URTSMoraleComponent>())
		{
			Handle.Snapshot.Morale = MC->CurrentMorale;
		}
		Handle.Snapshot.Discipline = 50.f;

		Group.Members.Add(Handle);
	}

	// Majority-vote for bIsRanged: group is ranged if more than half its members are Ranged role.
	int32 RangedCount = 0;
	for (const FCombatUnitHandle& H : Group.Members)
	{
		if (H.Snapshot.Role == ECombatRole::Ranged) ++RangedCount;
	}
	Group.bIsRanged = (Group.Members.Num() > 0 && RangedCount * 2 > Group.Members.Num());
	RefreshGroupState(Group);
	return Group;
}

float URTSCombatResolverLibrary::GetDamageTypeVsArmorMultiplier(EDamageTypeClass DamageType, EArmorClass ArmorClass)
{
	switch (DamageType)
	{
	case EDamageTypeClass::Blade:
		switch (ArmorClass)
		{
		case EArmorClass::Light: return 1.10f;
		case EArmorClass::Medium: return 1.00f;
		case EArmorClass::Heavy: return 0.90f;
		case EArmorClass::Monster: return 0.95f;
		case EArmorClass::Heroic: return 0.85f;
		default: return 1.00f;
		}
	case EDamageTypeClass::Pierce:
		switch (ArmorClass)
		{
		case EArmorClass::Light: return 1.20f;
		case EArmorClass::Medium: return 1.00f;
		case EArmorClass::Heavy: return 0.80f;
		case EArmorClass::Monster: return 0.85f;
		case EArmorClass::Heroic: return 0.80f;
		default: return 1.00f;
		}
	case EDamageTypeClass::Claw:
	case EDamageTypeClass::Bite:
		switch (ArmorClass)
		{
		case EArmorClass::Light: return 1.10f;
		case EArmorClass::Medium: return 1.05f;
		case EArmorClass::Heavy: return 0.95f;
		case EArmorClass::Monster: return 1.00f;
		case EArmorClass::Heroic: return 0.95f;
		default: return 1.00f;
		}
	case EDamageTypeClass::Magic:
		switch (ArmorClass)
		{
		case EArmorClass::Heavy: return 1.05f;
		case EArmorClass::Monster: return 1.10f;
		case EArmorClass::Heroic: return 1.00f;
		default: return 1.00f;
		}
	case EDamageTypeClass::Siege:
		return ArmorClass == EArmorClass::Structure ? 1.50f : 0.85f;
	default:
		return 1.00f;
	}
}

float URTSCombatResolverLibrary::ComputeMoraleModifier(float Morale)
{
	return FMath::Clamp(0.50f + (Morale / 100.f) * 0.85f, GMinMoraleModifier, GMaxMoraleModifier);
}

float URTSCombatResolverLibrary::ComputeGroupResolvePower(const FCombatGroup& Group, const FCombatSide& OwningSide)
{
	if (Group.LivingCount <= 0) return 0.f;

	const float UnitDensityPower = Group.LivingCount * Group.AvgAttack;
	const float TempoPower = FMath::Lerp(0.85f, 1.15f, FMath::Clamp(Group.AvgAttackSpeed / 1.25f, 0.f, 1.f));
	const float MoralePower = ComputeMoraleModifier(Group.AvgMorale + OwningSide.HeroData.MoraleBonus);
	const float LeadershipPower = GetHeroBonus(OwningSide) * OwningSide.LeadershipBonus;
	const float RolePower = Group.bIsRanged ? 0.95f : 1.0f;
	const float SustainPower = 1.f + Group.AvgLifeSteal * 0.15f;

	return UnitDensityPower * TempoPower * MoralePower * LeadershipPower
		* Group.FormationBonus * Group.TerrainBonus * Group.FlankBonus * Group.FatiguePenalty
		* Group.HeroBonus * RolePower * SustainPower;
}

float URTSCombatResolverLibrary::ComputeDefenseMitigation(const FCombatGroup& Defender)
{
	const float BaseMitigation = 1.f + (Defender.AvgDefense * GDefenseWeight / 100.f);
	const float DisciplineMitigation = 1.f + (Defender.AvgDiscipline / 250.f);
	return BaseMitigation * DisciplineMitigation;
}

int32 URTSCombatResolverLibrary::ChooseTargetGroupIndex(const FCombatGroup& Attacker, const FCombatSide& EnemySide)
{
	int32 BestIndex = INDEX_NONE;
	float BestScore = -FLT_MAX;

	for (int32 Index = 0; Index < EnemySide.Groups.Num(); ++Index)
	{
		const FCombatGroup& Candidate = EnemySide.Groups[Index];
		if (Candidate.LivingCount <= 0) continue;

		float Score = 0.f;
		Score += (Candidate.Role == ECombatRole::Frontline) ? 25.f : 0.f;
		Score += (Candidate.Role == ECombatRole::Ranged && Attacker.Role != ECombatRole::Frontline) ? 12.f : 0.f;
		Score += FMath::Clamp(100.f - Candidate.AvgMorale, 0.f, 40.f) * 0.3f;
		Score += FMath::Clamp(12.f - Candidate.AvgDefense, 0.f, 12.f) * 0.8f;
		Score += (Candidate.Role == ECombatRole::Hero) ? 8.f : 0.f;
		Score += (Attacker.Role == ECombatRole::Monster && Candidate.Role == ECombatRole::Ranged) ? 10.f : 0.f;

		if (Score > BestScore)
		{
			BestScore = Score;
			BestIndex = Index;
		}
	}
	return BestIndex;
}

FCombatRoundResult URTSCombatResolverLibrary::ResolveCombatRound(FCombatInstance& Combat, int32 RandomSeed)
{
	FCombatRoundResult Result;
	FRandomStream RNG(RandomSeed);

	auto ResolveAttacks = [&](FCombatSide& AttackingSide, FCombatSide& DefendingSide, int32& OutKills)
		{
		for (FCombatGroup& Attacker : AttackingSide.Groups)
		{
			if (Attacker.LivingCount <= 0) continue;

			const int32 TargetIndex = ChooseTargetGroupIndex(Attacker, DefendingSide);
			if (!DefendingSide.Groups.IsValidIndex(TargetIndex)) continue;

			FCombatGroup& Defender = DefendingSide.Groups[TargetIndex];
			const float AttackPower = ComputeGroupResolvePower(Attacker, AttackingSide);
			const float TypeMultiplier = GetDamageTypeVsArmorMultiplier(Attacker.DamageType, Defender.ArmorClass);
			const float PenetrationBoost = 1.f + (Attacker.AvgArmorPenetration * GArmorPenWeight / 100.f);
			const float RandomFactor = RNG.FRandRange(0.92f, 1.08f);
			const float RawDamage = AttackPower * TypeMultiplier * PenetrationBoost * RandomFactor;
			const float MitigatedDamage = RawDamage / ComputeDefenseMitigation(Defender);

			// Apply damage per unit (gradual HP decrease) instead of instant kills
			const int32 Kills = ApplyDamageToGroup(Defender, MitigatedDamage);
			OutKills += Kills;
		}
	};

	ResolveAttacks(Combat.SideA, Combat.SideB, Result.EstimatedKillsToSideB);
	ResolveAttacks(Combat.SideB, Combat.SideA, Result.EstimatedKillsToSideA);

	ApplyRoundMorale(Combat.SideA, Result.EstimatedKillsToSideA, Result.EstimatedKillsToSideB);
	ApplyRoundMorale(Combat.SideB, Result.EstimatedKillsToSideB, Result.EstimatedKillsToSideA);

	Result.MoraleDeltaSideA = Combat.SideA.SideMorale;
	Result.MoraleDeltaSideB = Combat.SideB.SideMorale;
	Result.bSideARouted = ShouldSideRout(Combat.SideA);
	Result.bSideBRouted = ShouldSideRout(Combat.SideB);

	if (Result.bSideARouted) Combat.SideA.State = ECombatState::Routed;
	if (Result.bSideBRouted) Combat.SideB.State = ECombatState::Routed;

	return Result;
}

int32 URTSCombatResolverLibrary::ApplyDamageToGroup(FCombatGroup& Group, float TotalDamage)
{
	if (TotalDamage <= 0.f || Group.LivingCount <= 0) return 0;

	// Frontline-first damage allocation: concentrate damage on lowest-HP units first
	// to eliminate them cleanly rather than spreading thin (prevents overkill waste).
	// Sort living handles by ascending CurrentHP so weakest absorb first.
	TArray<FCombatUnitHandle*> LivingHandles;
	LivingHandles.Reserve(Group.LivingCount);
	for (FCombatUnitHandle& Handle : Group.Members)
	{
		if (Handle.bAlive) LivingHandles.Add(&Handle);
	}
	LivingHandles.Sort([](const FCombatUnitHandle* A, const FCombatUnitHandle* B)
	{
		return A->Snapshot.CurrentHP < B->Snapshot.CurrentHP;
	});

	float RemainingDamage = TotalDamage;
	int32 Kills = 0;

	for (FCombatUnitHandle* Handle : LivingHandles)
	{
		if (RemainingDamage <= 0.f) break;

		const float AbsorbedDamage = FMath::Min(Handle->Snapshot.CurrentHP, RemainingDamage);
		RemainingDamage -= AbsorbedDamage;
		const float NewHP = Handle->Snapshot.CurrentHP - AbsorbedDamage;
		Handle->Snapshot.CurrentHP = NewHP;

		ARTSUnitCharacter* Unit = Handle->Unit.Get();
		if (Unit && IsValid(Unit))
		{
			Unit->OverrideHP = NewHP;  // Sync to unit so UI shows current HP
			if (NewHP <= 0.f)
			{
				Handle->bAlive = false;
				Kills++;
				UE_LOG(LogTemp, Log, TEXT("[RTS|Combat] Unit %s killed in combat (frontline-first allocation)."), *Unit->GetName());
				Unit->Destroy();
			}
		}
	}

	RefreshGroupState(Group);
	return Kills;
}

void URTSCombatResolverLibrary::RefreshGroupState(FCombatGroup& Group)
{
	Group.StartingCount = Group.Members.Num();
	Group.LivingCount = 0;
	Group.TotalHP = 0.f;
	Group.AvgAttack = 0.f;
	Group.AvgDefense = 0.f;
	Group.AvgAttackSpeed = 0.f;
	Group.AvgRange = 0.f;
	Group.AvgSpeed = 0.f;
	Group.AvgMorale = 0.f;
	Group.AvgDiscipline = 0.f;
	Group.AvgArmorPenetration = 0.f;
	Group.AvgLifeSteal = 0.f;
	Group.AvgFear = 0.f;

	for (const FCombatUnitHandle& Handle : Group.Members)
	{
		if (!Handle.bAlive) continue;
		Group.LivingCount++;
		Group.TotalHP += Handle.Snapshot.CurrentHP;
		Group.AvgAttack += Handle.Snapshot.Attack;
		Group.AvgDefense += Handle.Snapshot.Defense;
		Group.AvgAttackSpeed += Handle.Snapshot.AttackSpeed;
		Group.AvgRange += Handle.Snapshot.Range;
		Group.AvgSpeed += Handle.Snapshot.Speed;
		Group.AvgMorale += Handle.Snapshot.Morale;
		Group.AvgDiscipline += Handle.Snapshot.Discipline;
		Group.AvgArmorPenetration += Handle.Snapshot.ArmorPenetration;
		Group.AvgLifeSteal += Handle.Snapshot.LifeSteal;
		Group.AvgFear += Handle.Snapshot.Fear;
	}

	if (Group.LivingCount > 0)
	{
		const float InvCount = 1.f / static_cast<float>(Group.LivingCount);
		Group.AvgAttack *= InvCount;
		Group.AvgDefense *= InvCount;
		Group.AvgAttackSpeed *= InvCount;
		Group.AvgRange *= InvCount;
		Group.AvgSpeed *= InvCount;
		Group.AvgMorale *= InvCount;
		Group.AvgDiscipline *= InvCount;
		Group.AvgArmorPenetration *= InvCount;
		Group.AvgLifeSteal *= InvCount;
		Group.AvgFear *= InvCount;
	}
}

bool URTSCombatResolverLibrary::ShouldSideRout(const FCombatSide& Side)
{
	int32 StartingCount = 0;
	for (const FCombatGroup& G : Side.Groups) StartingCount += G.StartingCount;
	int32 CurrentCount = 0;
	for (const FCombatGroup& G : Side.Groups) CurrentCount += G.LivingCount;
	const float CasualtyRatio = StartingCount > 0 ? 1.f - (static_cast<float>(CurrentCount) / StartingCount) : 1.f;
	return Side.SideMorale <= 18.f || CasualtyRatio >= 0.60f;
}

float URTSCombatResolverLibrary::GetHeroBonus(const FCombatSide& Side)
{
	if (!Side.HeroData.bPresent) return 1.f;
	return Side.HeroData.LeadershipBonus * Side.HeroData.AuraAttackBonus;
}

void URTSCombatResolverLibrary::ApplyRoundMorale(FCombatSide& Side, int32 CasualtiesTaken, int32 CasualtiesInflicted)
{
	const float CasualtyPenalty = CasualtiesTaken * GCasualtyShock;
	const float KillReward = CasualtiesInflicted * GKillMoraleReward;
	const float HeroSupport = Side.HeroData.bPresent ? Side.HeroData.MoraleBonus : 0.f;
	const float NextMorale = Side.SideMorale - CasualtyPenalty + KillReward + HeroSupport * 0.10f;
	Side.SideMorale = FMath::Clamp(NextMorale, 0.f, 100.f);

	for (FCombatGroup& Group : Side.Groups)
	{
		Group.AvgMorale = FMath::Clamp(Group.AvgMorale - CasualtyPenalty * 0.15f + KillReward * 0.08f + HeroSupport * 0.05f, 0.f, 100.f);
	}
}
