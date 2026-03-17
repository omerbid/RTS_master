#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RTSProjectIndexSaveGame.h"
#include "RTSSaveSubsystem.generated.h"

/**
 * Single entry point for Save/Load ("Existing Project" mode).
 * Docs: save_load_spec.md, existing_project_mode_spec.md.
 * Stub implementation: API present, minimal behavior (no disk I/O yet).
 */
UCLASS()
class RTS_MONSTERS_API URTSSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Acquire save lock (no concurrent Save/Load). Returns true if acquired. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Save")
	bool AcquireSaveLock();

	/** Release save lock. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Save")
	void ReleaseSaveLock();

	/** Request manual save for project. Stub: acquires lock, releases. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Save")
	void RequestManualSave(FGuid ProjectId);

	/** Request load for project. Stub: acquires lock, releases. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Save")
	void RequestLoad(FGuid ProjectId);

	/** List projects from index. Stub: returns empty array. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Save")
	TArray<struct FProjectMeta> ListProjects();

	/** Create new project entry. Stub: returns new GUID. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Save")
	FGuid CreateProject();

	/** Delete project. Stub: no-op. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Save")
	void DeleteProject(FGuid ProjectId);

	/** True while Save/Load in progress (lock held). */
	UFUNCTION(BlueprintPure, Category = "RTS|Save")
	bool IsSaveLocked() const { return bSaveLock; }

private:
	bool bSaveLock = false;
};
