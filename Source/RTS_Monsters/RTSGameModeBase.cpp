#include "RTSGameModeBase.h"

#include "RTSPlayerController.h"
#include "RTSCameraPawn.h"

ARTSGameModeBase::ARTSGameModeBase()
{
	PlayerControllerClass = ARTSPlayerController::StaticClass();
	DefaultPawnClass = ARTSCameraPawn::StaticClass();
}
