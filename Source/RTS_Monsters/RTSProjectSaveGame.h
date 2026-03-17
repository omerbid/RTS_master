#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "RTSProjectSaveGame.generated.h"

/**
 * SaveGame for a single project's tactical/world state (v1 baseline).
 * Full schema in Docs/save_load_spec.md and save_migration_policy.md.
 * This is a minimal stub for Vertical Slice; extend with Regions, Units, Economy, Victory, etc.
 */
UCLASS()
class RTS_MONSTERS_API URTSProjectSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save|Meta")
	int32 SaveSchemaVersion = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save|Meta")
	FGuid ProjectId;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save|Meta")
	FString ProjectDisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save|Meta")
	FName MapName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save|Meta")
	FDateTime CreatedUtc;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save|Meta")
	FDateTime LastSavedUtc;
};
