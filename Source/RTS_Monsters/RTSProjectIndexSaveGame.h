#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "RTSProjectIndexSaveGame.generated.h"

/** Per-project metadata for "Existing Project" list. Save/Load spec: Index layer. */
USTRUCT(BlueprintType)
struct FProjectMeta
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ProjectId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ProjectDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastSavedUtc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SchemaVersion = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNeedsMigration = false;
};

/**
 * SaveGame for the project index: list of projects + metadata.
 * Used by URTSSaveSubsystem for "Existing Project" menu and slot selection (A/B).
 */
UCLASS()
class RTS_MONSTERS_API URTSProjectIndexSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save")
	TArray<FProjectMeta> Projects;
};
