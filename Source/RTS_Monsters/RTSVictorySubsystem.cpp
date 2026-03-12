#include "RTSVictorySubsystem.h"

#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "RTSHeroCharacter.h"
#include "RTSRegionVolume.h"

void URTSVictorySubsystem::NotifyControlReachedFive(ARTSRegionVolume* Region, EFactionId Faction)
{
	if (GameResult != ERTSGameResult::Playing) return;
	GameResult = ERTSGameResult::Won;
	WinningFaction = Faction;
	OnGameWon.Broadcast(Faction);
}

void URTSVictorySubsystem::NotifyHeroDeath(ARTSHeroCharacter* Hero)
{
	if (!Hero || GameResult != ERTSGameResult::Playing) return;
	UWorld* World = Hero->GetWorld();
	if (!World) return;
	CheckLoseCondition(World, Hero->FactionId);
}

void URTSVictorySubsystem::CheckLoseCondition(UWorld* World, EFactionId Faction)
{
	if (GameResult != ERTSGameResult::Playing) return;

	// Any living Hero of this faction?
	bool bHasLivingHero = false;
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(World, ARTSHeroCharacter::StaticClass(), Found);
	for (AActor* A : Found)
	{
		if (!IsValid(A)) continue;
		ARTSHeroCharacter* H = Cast<ARTSHeroCharacter>(A);
		if (H && H->FactionId == Faction)
		{
			bHasLivingHero = true;
			break;
		}
	}
	if (bHasLivingHero) return;

	// Any region with control >= 3 for this faction?
	bool bHasRegionWithControl3 = false;
	for (TActorIterator<ARTSRegionVolume> It(World); It; ++It)
	{
		ARTSRegionVolume* R = *It;
		if (IsValid(R) && R->GetControlLevelForFaction(Faction) >= 3)
		{
			bHasRegionWithControl3 = true;
			break;
		}
	}
	if (bHasRegionWithControl3) return;

	// GDD: no living hero and no region >= 3 -> lose
	GameResult = ERTSGameResult::Lost;
	LosingFaction = Faction;
	OnGameLost.Broadcast(Faction);
}
