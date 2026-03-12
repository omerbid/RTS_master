#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RTSCommandAuthorityComponent.generated.h"

class ARTSUnitCharacter;

/**
 * Command authority: validates whether an issuing actor can give new orders to a unit.
 * Does not perform the order itself; only range/faction checks.
 */
UCLASS(ClassGroup = (RTS), meta = (BlueprintSpawnableComponent))
class RTS_MONSTERS_API URTSCommandAuthorityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URTSCommandAuthorityComponent();

	/** Command radius in Unreal Units (Hero / Captain specific values set per owner). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Command")
	float CommandRadius;

	/** Whether this authority is currently active (e.g. owner alive, not stunned, etc.). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Command")
	bool bAuthorityEnabled;

	/** Can the owner issue a new order to the given unit? */
	UFUNCTION(BlueprintCallable, Category = "Command")
	bool CanIssueOrderToUnit(const ARTSUnitCharacter* TargetUnit) const;

protected:
	virtual void BeginPlay() override;
};

