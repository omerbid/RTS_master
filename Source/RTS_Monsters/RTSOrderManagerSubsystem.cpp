#include "RTSOrderManagerSubsystem.h"

#include "RTSUnitCharacter.h"
#include "RTSOrderComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

void URTSOrderManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void URTSOrderManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void URTSOrderManagerSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateAllUnitsWithOrders(DeltaTime);
}

TStatId URTSOrderManagerSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URTSOrderManagerSubsystem, STATGROUP_Tickables);
}

void URTSOrderManagerSubsystem::UpdateAllUnitsWithOrders(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ARTSUnitCharacter> It(World); It; ++It)
	{
		ARTSUnitCharacter* Unit = *It;
		if (!Unit || !IsValid(Unit))
		{
			continue;
		}

		URTSOrderComponent* OrderComp = Unit->FindComponentByClass<URTSOrderComponent>();
		if (OrderComp && OrderComp->HasOrder())
		{
			OrderComp->ExecuteOrder(DeltaTime);
		}
	}
}
