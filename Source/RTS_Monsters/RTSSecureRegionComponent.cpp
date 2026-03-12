#include "RTSSecureRegionComponent.h"

#include "RTSHeroCharacter.h"
#include "RTSRegionVolume.h"
#include "Engine/World.h"
#include "TimerManager.h"

URTSSecureRegionComponent::URTSSecureRegionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URTSSecureRegionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void URTSSecureRegionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CancelSecureRegion();
	Super::EndPlay(EndPlayReason);
}

bool URTSSecureRegionComponent::CanStartSecureRegion() const
{
	ARTSHeroCharacter* Hero = Cast<ARTSHeroCharacter>(GetOwner());
	if (!Hero || !GetWorld()) return false;

	ARTSRegionVolume* Region = ARTSRegionVolume::GetRegionAtLocation(this, Hero->GetActorLocation());
	if (!Region || !IsValid(Region)) return false;

	// GDD: available at control 4, Hero in region, not contested
	if (Region->GetControlLevelForFaction(Hero->FactionId) != 4) return false;
	if (Region->IsContested()) return false;
	if (!Region->IsPointInRegion(Hero->GetActorLocation())) return false;

	return true;
}

bool URTSSecureRegionComponent::StartSecureRegion()
{
	if (bIsChannelActive) return false;

	ARTSHeroCharacter* Hero = Cast<ARTSHeroCharacter>(GetOwner());
	if (!Hero || !GetWorld()) return false;

	ARTSRegionVolume* Region = ARTSRegionVolume::GetRegionAtLocation(this, Hero->GetActorLocation());
	if (!Region || !IsValid(Region)) return false;

	if (Region->GetControlLevelForFaction(Hero->FactionId) != 4) return false;
	if (Region->IsContested()) return false;
	if (!Region->IsPointInRegion(Hero->GetActorLocation())) return false;

	SecuringRegion = Region;
	bIsChannelActive = true;
	ChannelElapsedSeconds = 0.f;

	FTimerManager& TM = GetWorld()->GetTimerManager();
	TM.SetTimer(ValidationTimerHandle, this, &URTSSecureRegionComponent::ValidateAndTickChannel,
		ValidationIntervalSeconds, true);

	OnSecureRegionStarted.Broadcast();
	return true;
}

void URTSSecureRegionComponent::CancelSecureRegion()
{
	if (!bIsChannelActive) return;

	FTimerManager& TM = GetWorld()->GetTimerManager();
	TM.ClearTimer(ValidationTimerHandle);
	TM.ClearTimer(ChannelTimerHandle);

	bIsChannelActive = false;
	SecuringRegion = nullptr;
	ChannelElapsedSeconds = 0.f;

	OnSecureRegionCancelled.Broadcast();
}

void URTSSecureRegionComponent::ValidateAndTickChannel()
{
	if (!bIsChannelActive || !GetWorld()) return;

	ARTSHeroCharacter* Hero = Cast<ARTSHeroCharacter>(GetOwner());
	ARTSRegionVolume* Region = SecuringRegion.Get();

	if (!Hero || !IsValid(Hero) || !Region || !IsValid(Region))
	{
		CancelSecureRegion();
		return;
	}

	// GDD: cancel if Hero leaves region or enemy Hero enters
	if (!Region->IsPointInRegion(Hero->GetActorLocation()))
	{
		CancelSecureRegion();
		return;
	}
	if (Region->IsContested())
	{
		CancelSecureRegion();
		return;
	}

	ChannelElapsedSeconds += ValidationIntervalSeconds;
	if (ChannelElapsedSeconds >= SecureChannelDurationSeconds)
	{
		GetWorld()->GetTimerManager().ClearTimer(ValidationTimerHandle);
		CompleteChannel();
	}
}

void URTSSecureRegionComponent::CompleteChannel()
{
	if (!bIsChannelActive) return;

	ARTSHeroCharacter* Hero = Cast<ARTSHeroCharacter>(GetOwner());
	ARTSRegionVolume* Region = SecuringRegion.Get();

	if (Hero && IsValid(Hero) && Region && IsValid(Region))
	{
		Region->SetControlLevelForFaction(Hero->FactionId, 5);
		OnSecureRegionCompleted.Broadcast(Region);
	}

	GetWorld()->GetTimerManager().ClearTimer(ValidationTimerHandle);
	bIsChannelActive = false;
	SecuringRegion = nullptr;
	ChannelElapsedSeconds = 0.f;
}
