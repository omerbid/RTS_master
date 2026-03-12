#include "RTSCameraPawn.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"

ARTSCameraPawn::ARTSCameraPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 2000.f;
	SpringArm->SetRelativeRotation(FRotator(-80.f, 0.f, 0.f));
	SpringArm->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
	Movement->MaxSpeed = 1200.f;
	Movement->Acceleration = 2000.f;
	Movement->Deceleration = 4000.f;
	Movement->bConstrainToPlane = true;
	Movement->SetPlaneConstraintNormal(FVector(0.f, 0.f, 1.f));
}

void ARTSCameraPawn::MoveForward(float Value)
{
	FVector Dir = GetActorForwardVector();
	Dir.Z = 0.f;
	Dir.Normalize();
	AddMovementInput(Dir, Value);
}

void ARTSCameraPawn::MoveRight(float Value)
{
	FVector Dir = GetActorRightVector();
	Dir.Z = 0.f;
	Dir.Normalize();
	AddMovementInput(Dir, Value);
}

void ARTSCameraPawn::ZoomInOut(float Value)
{
	if (!SpringArm || FMath::IsNearlyZero(Value)) return;
	const float Delta = -Value * 150.f;
	SpringArm->TargetArmLength = FMath::Clamp(SpringArm->TargetArmLength + Delta, ZoomMin, ZoomMax);
}

void ARTSCameraPawn::AdjustCameraPitch(float Value)
{
	if (!SpringArm || FMath::IsNearlyZero(Value)) return;
	FRotator Rot = SpringArm->GetRelativeRotation();
	Rot.Pitch = FMath::Clamp(Rot.Pitch + Value * 0.5f, PitchMin, PitchMax);
	SpringArm->SetRelativeRotation(Rot);
}

void ARTSCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UInputComponent* IC = PlayerInputComponent)
	{
		IC->BindAxis("MoveForward", this, &ARTSCameraPawn::MoveForward);
		IC->BindAxis("MoveRight", this, &ARTSCameraPawn::MoveRight);
	}
}
