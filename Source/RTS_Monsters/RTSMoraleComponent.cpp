#include "RTSMoraleComponent.h"

URTSMoraleComponent::URTSMoraleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	BaseMorale = 60.f;
	CurrentMorale = BaseMorale;
	MinMorale = 0.f;
	MaxMorale = 100.f;
}

void URTSMoraleComponent::BeginPlay()
{
	Super::BeginPlay();

	// Ensure initial value is clamped.
	SetMorale(CurrentMorale);
}

float URTSMoraleComponent::ApplyMoraleDelta(float Delta)
{
	const float NewValue = FMath::Clamp(CurrentMorale + Delta, MinMorale, MaxMorale);
	CurrentMorale = NewValue;
	return CurrentMorale;
}

void URTSMoraleComponent::SetMorale(float NewValue)
{
	CurrentMorale = FMath::Clamp(NewValue, MinMorale, MaxMorale);
}

