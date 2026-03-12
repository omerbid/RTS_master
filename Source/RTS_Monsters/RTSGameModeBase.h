#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RTSGameModeBase.generated.h"

/**
 * P1: Uses ARTSPlayerController and ARTSCameraPawn.
 * Set in map or Project Settings -> Default GameMode to enable selection, orders, and camera pan.
 */
UCLASS()
class RTS_MONSTERS_API ARTSGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARTSGameModeBase();
};
