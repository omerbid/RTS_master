#include "RTSSaveSubsystem.h"

bool URTSSaveSubsystem::AcquireSaveLock()
{
	if (bSaveLock)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RTS|Save] AcquireSaveLock FAILED – already locked."));
		return false;
	}
	bSaveLock = true;
	UE_LOG(LogTemp, Log, TEXT("[RTS|Save] SaveLock ACQUIRED."));
	return true;
}

void URTSSaveSubsystem::ReleaseSaveLock()
{
	bSaveLock = false;
	UE_LOG(LogTemp, Log, TEXT("[RTS|Save] SaveLock RELEASED."));
}

void URTSSaveSubsystem::RequestManualSave(FGuid ProjectId)
{
	UE_LOG(LogTemp, Log, TEXT("[RTS|Save] RequestManualSave project=%s"), *ProjectId.ToString());
	if (!AcquireSaveLock()) return;
	// TODO: Gather snapshot, AsyncSaveGameToSlot (A/B), Update Index, Validate.
	ReleaseSaveLock();
}

void URTSSaveSubsystem::RequestLoad(FGuid ProjectId)
{
	UE_LOG(LogTemp, Log, TEXT("[RTS|Save] RequestLoad project=%s"), *ProjectId.ToString());
	if (!AcquireSaveLock()) return;
	// TODO: AsyncLoadGameFromSlot, Migrate if needed, Rehydrate Phase 1+2.
	ReleaseSaveLock();
}

TArray<FProjectMeta> URTSSaveSubsystem::ListProjects()
{
	// TODO: Load IndexSaveGame from slot, return Projects array.
	return TArray<FProjectMeta>();
}

FGuid URTSSaveSubsystem::CreateProject()
{
	// TODO: Create entry in Index, optional initial ProjectSave.
	return FGuid::NewGuid();
}

void URTSSaveSubsystem::DeleteProject(FGuid ProjectId)
{
	// TODO: Remove from Index, delete slot files if desired.
}
