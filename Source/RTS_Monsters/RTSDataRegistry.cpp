#include "RTSDataRegistry.h"

#include "Engine/DataTable.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const TCHAR* PathUnits       = TEXT("/Game/RTS/Data/Data_Tables/DT_Units_MVP.DT_Units_MVP");
	const TCHAR* PathHeroes      = TEXT("/Game/RTS/Data/Data_Tables/DT_Heroes_MVP.DT_Heroes_MVP");
	const TCHAR* PathHeroXP      = TEXT("/Game/RTS/Data/Data_Tables/DT_HeroXP_Prototype.DT_HeroXP_Prototype");
	const TCHAR* PathMoraleThresh = TEXT("/Game/RTS/Data/Data_Tables/DT_MoraleThresholds.DT_MoraleThresholds");
}

void URTSDataRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadTablesFromPaths();
	ValidateAndLog();
}

void URTSDataRegistry::LoadTablesFromPaths()
{
	UnitsTable        = LoadObject<UDataTable>(nullptr, PathUnits);
	HeroesTable       = LoadObject<UDataTable>(nullptr, PathHeroes);
	HeroXPTable       = LoadObject<UDataTable>(nullptr, PathHeroXP);
	MoraleThresholdsTable = LoadObject<UDataTable>(nullptr, PathMoraleThresh);

	if (!UnitsTable)  { UE_LOG(LogTemp, Warning, TEXT("RTSDataRegistry: Units table not found at %s"), PathUnits); }
	if (!HeroesTable) { UE_LOG(LogTemp, Warning, TEXT("RTSDataRegistry: Heroes table not found at %s"), PathHeroes); }
	if (!HeroXPTable) { UE_LOG(LogTemp, Log, TEXT("RTSDataRegistry: HeroXP table not found at %s (optional)"), PathHeroXP); }
	if (!MoraleThresholdsTable) { UE_LOG(LogTemp, Log, TEXT("RTSDataRegistry: MoraleThresholds table not found at %s (optional)"), PathMoraleThresh); }
}

bool URTSDataRegistry::ValidateAndLog()
{
	bool bOk = true;
	if (UnitsTable)
	{
		TSet<FName> SeenIds;
		for (const auto& Pair : UnitsTable->GetRowMap())
		{
			const FUnitRow* Row = reinterpret_cast<const FUnitRow*>(Pair.Value);
			if (Row)
			{
				if (SeenIds.Contains(Row->UnitId))
				{
					UE_LOG(LogTemp, Error, TEXT("RTSDataRegistry: Duplicate UnitId: %s"), *Row->UnitId.ToString());
					bOk = false;
				}
				SeenIds.Add(Row->UnitId);
			}
		}
	}
	if (HeroesTable)
	{
		TSet<FName> SeenIds;
		for (const auto& Pair : HeroesTable->GetRowMap())
		{
			const FHeroRow* Row = reinterpret_cast<const FHeroRow*>(Pair.Value);
			if (Row)
			{
				if (SeenIds.Contains(Row->HeroId))
				{
					UE_LOG(LogTemp, Error, TEXT("RTSDataRegistry: Duplicate HeroId: %s"), *Row->HeroId.ToString());
					bOk = false;
				}
				SeenIds.Add(Row->HeroId);
			}
		}
	}
	return bOk;
}

bool URTSDataRegistry::GetUnitRow(FName UnitId, FUnitRow& OutRow) const
{
	if (!UnitsTable)
	{
		return false;
	}
	for (const auto& Pair : UnitsTable->GetRowMap())
	{
		const FUnitRow* Row = reinterpret_cast<const FUnitRow*>(Pair.Value);
		if (Row && Row->UnitId == UnitId)
		{
			OutRow = *Row;
			return true;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("RTSDataRegistry: UnitId not found: %s"), *UnitId.ToString());
	return false;
}

bool URTSDataRegistry::GetHeroRow(FName HeroId, FHeroRow& OutRow) const
{
	if (!HeroesTable)
	{
		return false;
	}
	for (const auto& Pair : HeroesTable->GetRowMap())
	{
		const FHeroRow* Row = reinterpret_cast<const FHeroRow*>(Pair.Value);
		if (Row && Row->HeroId == HeroId)
		{
			OutRow = *Row;
			return true;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("RTSDataRegistry: HeroId not found: %s"), *HeroId.ToString());
	return false;
}

bool URTSDataRegistry::GetHeroXPRow(FName TableId, int32 Level, FHeroXPRow& OutRow) const
{
	if (!HeroXPTable)
	{
		return false;
	}
	const TMap<FName, uint8*>& RowMap = HeroXPTable->GetRowMap();
	for (const auto& Pair : RowMap)
	{
		const FHeroXPRow* Row = reinterpret_cast<const FHeroXPRow*>(Pair.Value);
		if (Row && Row->TableId == TableId && Row->Level == Level)
		{
			OutRow = *Row;
			return true;
		}
	}
	return false;
}

bool URTSDataRegistry::GetMoraleThresholdRow(FName Name, FMoraleThresholdRow& OutRow) const
{
	if (!MoraleThresholdsTable)
	{
		return false;
	}
	const FMoraleThresholdRow* Row = MoraleThresholdsTable->FindRow<FMoraleThresholdRow>(Name, TEXT("GetMoraleThresholdRow"));
	if (!Row)
	{
		return false;
	}
	OutRow = *Row;
	return true;
}

bool URTSDataRegistry::GetUnitRowForHero(const FHeroRow& HeroRow, FUnitRow& OutUnitRow) const
{
	// Build a synthetic unit row from hero data so we don't depend on a specific unit row.
	OutUnitRow.Faction = HeroRow.Faction;
	OutUnitRow.UnitId = HeroRow.HeroId;
	OutUnitRow.DisplayName = HeroRow.DisplayName;
	OutUnitRow.Role = EUnitRole::MeleeMedium;
	OutUnitRow.PowerScale = HeroRow.PowerScale;
	OutUnitRow.HP = HeroRow.BaseHP;
	OutUnitRow.Damage = HeroRow.BaseDamage;
	OutUnitRow.MoveSpeed = HeroRow.MoveSpeed;
	OutUnitRow.MoraleBase = 70.f;
	OutUnitRow.Upkeep = 0;
	OutUnitRow.Special = ESpecialTag::None;
	OutUnitRow.Rank = 1; // Heroes are not promoted to Captain.
	return true;
}
