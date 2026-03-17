#include "RTSDayNightSubsystem.h"

#include "Engine/World.h"
#include "TimerManager.h"

void URTSDayNightSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	bIsNight = bStartAtNight;
	ElapsedInPhase = 0.f;

	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (World)
	{
		FTimerDelegate Delegate;
		Delegate.BindUObject(this, &URTSDayNightSubsystem::TickPhase, 1.f);
		World->GetTimerManager().SetTimer(TimerHandlePhase, Delegate, 1.f, true, 0.f);
	}
}

void URTSDayNightSubsystem::Deinitialize()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UWorld* World = GI->GetWorld())
		{
			World->GetTimerManager().ClearTimer(TimerHandlePhase);
		}
	}
	Super::Deinitialize();
}

float URTSDayNightSubsystem::GetSecondsUntilNextPhase() const
{
	return FMath::Max(0.f, PhaseDurationSeconds - ElapsedInPhase);
}

void URTSDayNightSubsystem::TickPhase(float DeltaTime)
{
	if (PhaseDurationSeconds <= 0.f) return;

	ElapsedInPhase += DeltaTime;
	if (ElapsedInPhase >= PhaseDurationSeconds)
	{
		ElapsedInPhase = 0.f;
		bIsNight = !bIsNight;
		OnDayNightPhaseChanged.Broadcast(bIsNight ? ERTSDayNightPhase::Night : ERTSDayNightPhase::Day);
	}
}
