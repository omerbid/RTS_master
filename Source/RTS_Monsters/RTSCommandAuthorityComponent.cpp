#include "RTSCommandAuthorityComponent.h"

#include "GameFramework/Actor.h"
#include "RTSUnitCharacter.h"

URTSCommandAuthorityComponent::URTSCommandAuthorityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	CommandRadius = 2500.f;
	bAuthorityEnabled = true;
}

void URTSCommandAuthorityComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool URTSCommandAuthorityComponent::CanIssueOrderToUnit(const ARTSUnitCharacter* TargetUnit) const
{
	if (!bAuthorityEnabled || !TargetUnit)
	{
		return false;
	}

	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	// Same-faction check: issuer (owner of this component) must be same faction as target unit.
	const ARTSUnitCharacter* IssuerUnit = Cast<ARTSUnitCharacter>(OwnerActor);
	if (IssuerUnit && IssuerUnit->FactionId != TargetUnit->FactionId)
	{
		return false;
	}

	const float DistanceSq = FVector::DistSquared(OwnerActor->GetActorLocation(), TargetUnit->GetActorLocation());
	const float RadiusSq = CommandRadius * CommandRadius;

	return DistanceSq <= RadiusSq;
}

