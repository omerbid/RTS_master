#include "RTSRegionVolume.h"

#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "RTSHeroCharacter.h"
#include "RTSUnitCharacter.h"
#include "RTSVictorySubsystem.h"

ARTSRegionVolume::ARTSRegionVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	RegionBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("RegionBounds"));
	RootComponent = RegionBounds;

	ControlLevelHumans = 0;
	ControlLevelVampires = 0;
	ControlLevelWerewolves = 0;

	Population = 100;  // P4: Default for Vampire/Werewolf conversion; set in level for balance
	Stability = 50.f;
	DominantFaction = EFactionId::Humans;
}

void ARTSRegionVolume::BeginPlay()
{
	Super::BeginPlay();

	if (RegionBounds)
	{
		RegionBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		RegionBounds->SetGenerateOverlapEvents(true);
		RegionBounds->OnComponentBeginOverlap.AddDynamic(this, &ARTSRegionVolume::OnRegionBoundsBeginOverlap);
		RegionBounds->OnComponentEndOverlap.AddDynamic(this, &ARTSRegionVolume::OnRegionBoundsEndOverlap);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ControlGainTimerHandle, this, &ARTSRegionVolume::EvaluateControlGain,
			ControlGainIntervalSeconds, true);
	}
}

bool ARTSRegionVolume::IsPointInRegion(FVector WorldLocation) const
{
	if (!RegionBounds) return false;
	FBox Box = RegionBounds->Bounds.GetBox();
	return FMath::PointBoxIntersection(WorldLocation, Box);
}

void ARTSRegionVolume::SetControlLevelForFaction(EFactionId Faction, int32 Level)
{
	const int32 Clamped = FMath::Clamp(Level, 0, 5);
	switch (Faction)
	{
	case EFactionId::Humans:
		ControlLevelHumans = Clamped;
		break;
	case EFactionId::Vampires:
		ControlLevelVampires = Clamped;
		break;
	case EFactionId::Werewolves:
		ControlLevelWerewolves = Clamped;
		break;
	default:
		return;
	}
	RecalcDominantFaction();

	if (Clamped >= 5)
	{
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				if (URTSVictorySubsystem* Victory = GI->GetSubsystem<URTSVictorySubsystem>())
				{
					Victory->NotifyControlReachedFive(this, Faction);
				}
			}
		}
	}
}

void ARTSRegionVolume::RecalcDominantFaction()
{
	int32 BestLevel = ControlLevelHumans;
	EFactionId Best = EFactionId::Humans;
	if (ControlLevelVampires > BestLevel) { BestLevel = ControlLevelVampires; Best = EFactionId::Vampires; }
	if (ControlLevelWerewolves > BestLevel) { BestLevel = ControlLevelWerewolves; Best = EFactionId::Werewolves; }
	DominantFaction = Best;
}

int32 ARTSRegionVolume::GetControlLevelForFaction(EFactionId Faction) const
{
	switch (Faction)
	{
	case EFactionId::Humans:
		return ControlLevelHumans;
	case EFactionId::Vampires:
		return ControlLevelVampires;
	case EFactionId::Werewolves:
		return ControlLevelWerewolves;
	default:
		return 0;
	}
}

void ARTSRegionVolume::OnRegionBoundsBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;
	if (ARTSHeroCharacter* Hero = Cast<ARTSHeroCharacter>(OtherActor))
	{
		ActorsInRegion.Add(OtherActor);
		if (GEngine)
		{
			const FString FactionStr = Hero->FactionId == EFactionId::Humans ? TEXT("Humans") : Hero->FactionId == EFactionId::Vampires ? TEXT("Vampires") : TEXT("Werewolves");
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 4.f, FColor::Cyan, FString::Printf(TEXT("[Region] Hero entered region: %s (Faction: %s)"), *Hero->GetName(), *FactionStr));
		}
	}
	else if (Cast<ARTSUnitCharacter>(OtherActor))
	{
		ActorsInRegion.Add(OtherActor);
	}
}

void ARTSRegionVolume::OnRegionBoundsEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		ActorsInRegion.Remove(OtherActor);
	}
}

TArray<ARTSHeroCharacter*> ARTSRegionVolume::GetHeroesInRegion() const
{
	TArray<ARTSHeroCharacter*> Out;
	for (const TWeakObjectPtr<AActor>& Ptr : ActorsInRegion)
	{
		AActor* A = Ptr.Get();
		if (!A || !IsValid(A)) continue;
		if (ARTSHeroCharacter* Hero = Cast<ARTSHeroCharacter>(A))
		{
			Out.Add(Hero);
		}
	}
	return Out;
}

bool ARTSRegionVolume::HasHeroOfFaction(EFactionId Faction) const
{
	for (ARTSHeroCharacter* Hero : GetHeroesInRegion())
	{
		if (Hero && Hero->FactionId == Faction) return true;
	}
	return false;
}

bool ARTSRegionVolume::IsContested() const
{
	// Contested if two or more factions have a hero in region.
	bool bHuman = HasHeroOfFaction(EFactionId::Humans);
	bool bVampire = HasHeroOfFaction(EFactionId::Vampires);
	bool bWerewolf = HasHeroOfFaction(EFactionId::Werewolves);
	const int32 Count = (bHuman ? 1 : 0) + (bVampire ? 1 : 0) + (bWerewolf ? 1 : 0);
	return Count >= 2;
}

void ARTSRegionVolume::EvaluateControlGain()
{
	// GDD: control increases when faction has Hero in region and region not contested. Cap at 4; 5 only via Secure.
	if (IsContested()) return;

	const int32 MaxGain = FMath::Clamp(ControlGainMaxLevel, 0, 5);
	if (HasHeroOfFaction(EFactionId::Humans) && GetControlLevelForFaction(EFactionId::Humans) < MaxGain)
	{
		const int32 NewLevel = GetControlLevelForFaction(EFactionId::Humans) + 1;
		SetControlLevelForFaction(EFactionId::Humans, NewLevel);
		if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 4.f, FColor::Silver, FString::Printf(TEXT("[Region] Control Humans → %d"), NewLevel));
	}
	if (HasHeroOfFaction(EFactionId::Vampires) && GetControlLevelForFaction(EFactionId::Vampires) < MaxGain)
	{
		const int32 NewLevel = GetControlLevelForFaction(EFactionId::Vampires) + 1;
		SetControlLevelForFaction(EFactionId::Vampires, NewLevel);
		if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 4.f, FColor::Silver, FString::Printf(TEXT("[Region] Control Vampires → %d"), NewLevel));
	}
	if (HasHeroOfFaction(EFactionId::Werewolves) && GetControlLevelForFaction(EFactionId::Werewolves) < MaxGain)
	{
		const int32 NewLevel = GetControlLevelForFaction(EFactionId::Werewolves) + 1;
		SetControlLevelForFaction(EFactionId::Werewolves, NewLevel);
		if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 4.f, FColor::Silver, FString::Printf(TEXT("[Region] Control Werewolves → %d"), NewLevel));
	}
}

FText ARTSRegionVolume::GetControlLevelName(int32 Level)
{
	switch (FMath::Clamp(Level, 0, 5))
	{
	case 0: return FText::FromString(TEXT("None"));
	case 1: return FText::FromString(TEXT("Presence"));
	case 2: return FText::FromString(TEXT("Influence"));
	case 3: return FText::FromString(TEXT("Control"));
	case 4: return FText::FromString(TEXT("Established"));
	case 5: return FText::FromString(TEXT("Dominance"));
	default: return FText::FromString(TEXT("None"));
	}
}

ARTSRegionVolume* ARTSRegionVolume::GetRegionAtLocation(const UObject* WorldContextObject, FVector WorldLocation)
{
	// Do not pass CDO or base UObject to engine: GetWorldFromContextObject may Cast to AActor internally and crash (Default__Object).
	if (!WorldContextObject ||
	    WorldContextObject->HasAnyFlags(RF_ClassDefaultObject) ||
	    WorldContextObject->GetClass() == UObject::StaticClass())
	{
		return nullptr;
	}
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World) return nullptr;

	for (TActorIterator<ARTSRegionVolume> It(World); It; ++It)
	{
		ARTSRegionVolume* R = *It;
		if (IsValid(R) && R->IsPointInRegion(WorldLocation))
		{
			return R;
		}
	}
	return nullptr;
}

