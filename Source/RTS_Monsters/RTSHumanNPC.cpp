#include "RTSHumanNPC.h"

#include "RTSRegionVolume.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ARTSHumanNPC::ARTSHumanNPC()
{
	PrimaryActorTick.bCanEverTick = false;

	// Simpler than units: no combat, no orders
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = 280.f;  // Slower than soldiers
	}
}

void ARTSHumanNPC::BeginPlay()
{
	Super::BeginPlay();

	if (!PersistentNpcGuid.IsValid())
	{
		PersistentNpcGuid = FGuid::NewGuid();
	}

	// If no HomeRegion set, try to find one at spawn location
	if (!HomeRegion.IsValid() && GetWorld())
	{
		ARTSRegionVolume* Region = ARTSRegionVolume::GetRegionAtLocation(this, GetActorLocation());
		if (Region)
		{
			HomeRegion = Region;
		}
	}

	UpdateStateFromCurrent();
}

void ARTSHumanNPC::SetNPCState(EHumanNPCState NewState)
{
	NPCState = NewState;
	UpdateStateFromCurrent();
}

bool ARTSHumanNPC::TryCapture()
{
	if (!CanBeCaptured())
	{
		return false;
	}
	SetNPCState(EHumanNPCState::Captured);
	return true;
}

bool ARTSHumanNPC::CanBeCaptured() const
{
	return NPCState != EHumanNPCState::Captured;
}

void ARTSHumanNPC::UpdateStateFromCurrent()
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(NPCState == EHumanNPCState::Captured ? MOVE_None : MOVE_Walking);
	}
}
