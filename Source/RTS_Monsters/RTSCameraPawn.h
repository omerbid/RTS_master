#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "RTSCameraPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UFloatingPawnMovement;

/**
 * P1 Epic 5: Camera pan only. Pawn moved by WASD/Arrows; no unit control.
 */
UCLASS(meta = (DisplayName = "RTS Camera Pawn"))
class RTS_MONSTERS_API ARTSCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	ARTSCameraPawn();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UFloatingPawnMovement* Movement;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void ZoomInOut(float Value);
	void AdjustCameraPitch(float Value);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "300", ClampMax = "6000"))
	float ZoomMin = 500.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "300", ClampMax = "6000"))
	float ZoomMax = 4000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "-90", ClampMax = "0"))
	float PitchMin = -85.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "-90", ClampMax = "0"))
	float PitchMax = -30.f;

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
};
