#include "RTSGameModeBase.h"

#include "RTSPlayerController.h"
#include "RTSCameraPawn.h"
#include "RTSHUD.h"

ARTSGameModeBase::ARTSGameModeBase()
{
	PlayerControllerClass = ARTSPlayerController::StaticClass();
	DefaultPawnClass = ARTSCameraPawn::StaticClass();
	HUDClass = ARTSHUD::StaticClass();
}
