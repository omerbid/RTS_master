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
	const float OldMorale = CurrentMorale;
	const float NewValue = FMath::Clamp(CurrentMorale + Delta, MinMorale, MaxMorale);
	CurrentMorale = NewValue;

	// Log threshold crossings for diagnostics (30 = Low, 15 = Critical).
	const bool bCrossedLow = OldMorale >= 30.f && CurrentMorale < 30.f;
	const bool bCrossedCritical = OldMorale >= 15.f && CurrentMorale < 15.f;
	if (bCrossedCritical)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RTS|Morale] %s crossed CRITICAL threshold (%.1f -> %.1f)."),
			GetOwner() ? *GetOwner()->GetName() : TEXT("?"), OldMorale, CurrentMorale);
	}
	else if (bCrossedLow)
	{
		UE_LOG(LogTemp, Log, TEXT("[RTS|Morale] %s crossed LOW threshold (%.1f -> %.1f)."),
			GetOwner() ? *GetOwner()->GetName() : TEXT("?"), OldMorale, CurrentMorale);
	}
	return CurrentMorale;
}

void URTSMoraleComponent::SetMorale(float NewValue)
{
	CurrentMorale = FMath::Clamp(NewValue, MinMorale, MaxMorale);
}

