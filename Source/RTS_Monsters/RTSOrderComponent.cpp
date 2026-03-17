#include "RTSOrderComponent.h"

#include "RTSDataRegistry.h"
#include "RTSUnitCharacter.h"
#include "RTSCombatManagerSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"

URTSOrderComponent::URTSOrderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;  // Orders executed by RTSOrderManagerSubsystem (centralized)
}

void URTSOrderComponent::BeginPlay()
{
	Super::BeginPlay();
	CachedUnit = Cast<ARTSUnitCharacter>(GetOwner());
}

void URTSOrderComponent::SetCurrentOrder(ERTSOrderType OrderType, const FRTSOrderPayload& Payload)
{
	CurrentOrderType = OrderType;
	CurrentOrderPayload = Payload;
}

void URTSOrderComponent::ClearOrder(bool bApplyPostCombatBehavior)
{
	CurrentOrderType = ERTSOrderType::None;
	CurrentOrderPayload = FRTSOrderPayload{};
	if (bApplyPostCombatBehavior)
	{
		ApplyPostCombatBehavior();
	}
}

void URTSOrderComponent::ExecuteOrder(float DeltaTime)
{
	switch (CurrentOrderType)
	{
	case ERTSOrderType::Move:
		TickMove(DeltaTime);
		break;
	case ERTSOrderType::Attack:
		TickAttack(DeltaTime);
		break;
	default:
		break;
	}
}

void URTSOrderComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ExecuteOrder(DeltaTime);
}

void URTSOrderComponent::TickMove(float DeltaTime)
{
	ARTSUnitCharacter* Unit = CachedUnit.Get();
	if (!Unit)
	{
		return;
	}

	const FVector Dest = CurrentOrderPayload.MoveDestination;
	const FVector Loc = Unit->GetActorLocation();
	const float DistSq = FVector::DistSquared(Loc, Dest);
	const float AcceptSq = MoveAcceptanceRadius * MoveAcceptanceRadius;

	if (DistSq <= AcceptSq)
	{
		// Move completed; post-combat applies only after combat/Attack ends.
		ClearOrder(false);
		return;
	}

	UCharacterMovementComponent* Movement = Unit->GetCharacterMovement();
	if (!Movement)
	{
		return;
	}

	const float Responsiveness = Unit->GetOrderResponsivenessMultiplier();
	FVector Dir = (Dest - Loc).GetSafeNormal2D();
	const float Speed = Movement->GetMaxSpeed() * Responsiveness * DeltaTime;
	// Direct move: unpossessed RTS units ignore AddInputVector/AddMovementInput; SetActorLocation always works
	FVector NewLoc = Loc + Dir * Speed;
	Unit->SetActorLocation(NewLoc, true);
}

void URTSOrderComponent::TickAttack(float DeltaTime)
{
	ARTSUnitCharacter* Unit = CachedUnit.Get();
	AActor* Target = CurrentOrderPayload.AttackTarget.Get();
	if (!Unit || !Target || !IsValid(Target))
	{
		// Target dead or invalid: combat ended.
		ClearOrder(true);
		return;
	}

	const FVector TargetLoc = Target->GetActorLocation();
	const FVector Loc = Unit->GetActorLocation();
	const float DistSq = FVector::DistSquared(Loc, TargetLoc);
	// Use MoveAcceptanceRadius as melee range for now; real attack range from data can be added later.
	const float AttackRangeSq = FMath::Square(Unit->CachedUnitData.Range > 0.f ? Unit->CachedUnitData.Range : MoveAcceptanceRadius);

	if (DistSq <= AttackRangeSq)
	{
		// In range: start combat if target is RTS unit and not already in combat
		ARTSUnitCharacter* TargetUnit = Cast<ARTSUnitCharacter>(Target);
		if (TargetUnit && TargetUnit->FactionId != Unit->FactionId)
		{
			UWorld* World = Unit->GetWorld();
			if (World)
			{
				if (URTSCombatManagerSubsystem* CombatMgr = World->GetSubsystem<URTSCombatManagerSubsystem>())
				{
					if (!CombatMgr->IsUnitInCombat(Unit) && !CombatMgr->IsUnitInCombat(TargetUnit))
					{
						TArray<ARTSUnitCharacter*> SideA, SideB;
						SideA.Add(Unit);
						SideB.Add(TargetUnit);
						CombatMgr->StartCombat(SideA, SideB, (Loc + TargetLoc) * 0.5f);
					}
				}
			}
		}
		return;
	}

	// Move toward target.
	UCharacterMovementComponent* Movement = Unit->GetCharacterMovement();
	if (Movement)
	{
		FVector Dir = (TargetLoc - Loc).GetSafeNormal2D();
		const float Speed = Movement->GetMaxSpeed() * DeltaTime;
		Unit->SetActorLocation(Loc + Dir * Speed, true);
	}
}

void URTSOrderComponent::ApplyPostCombatBehavior()
{
	ARTSUnitCharacter* Unit = CachedUnit.Get();
	if (!Unit)
	{
		return;
	}

	const ERTSPostCombatBehavior Behavior = Unit->CachedUnitData.PostCombatBehavior;

	switch (Behavior)
	{
	case ERTSPostCombatBehavior::Hold:
		// Already cleared; unit stops (no new order).
		break;
	case ERTSPostCombatBehavior::Advance:
		{
			// Advance: move forward from current position (e.g. 500 UU ahead in facing or last-attack direction).
			FVector Forward = Unit->GetActorForwardVector();
			Forward.Z = 0.f;
			Forward.Normalize();
			FRTSOrderPayload Payload;
			Payload.MoveDestination = Unit->GetActorLocation() + Forward * 500.f;
			SetCurrentOrder(ERTSOrderType::Move, Payload);
		}
		break;
	case ERTSPostCombatBehavior::Retreat:
		{
			// Retreat: move backward from current position.
			FVector Back = -Unit->GetActorForwardVector();
			Back.Z = 0.f;
			Back.Normalize();
			FRTSOrderPayload Payload;
			Payload.MoveDestination = Unit->GetActorLocation() + Back * 400.f;
			SetCurrentOrder(ERTSOrderType::Move, Payload);
		}
		break;
	default:
		break;
	}
}
