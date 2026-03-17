#include "RTSHUD.h"

#include "RTSPlayerController.h"

void ARTSHUD::DrawHUD()
{
	Super::DrawHUD();

	ARTSPlayerController* PC = Cast<ARTSPlayerController>(GetOwningPlayerController());
	if (!PC || !PC->IsBoxSelectActive())
	{
		return;
	}

	FVector2D PressPos = PC->GetBoxSelectPressPos();
	float MouseX = 0.f, MouseY = 0.f;
	if (!PC->GetMousePosition(MouseX, MouseY))
	{
		return;
	}

	FVector2D Min(FMath::Min(PressPos.X, MouseX), FMath::Min(PressPos.Y, MouseY));
	FVector2D Max(FMath::Max(PressPos.X, MouseX), FMath::Max(PressPos.Y, MouseY));
	const float W = FMath::Max(1.f, Max.X - Min.X);
	const float H = FMath::Max(1.f, Max.Y - Min.Y);

	// Gray semi-transparent box for box selection (user requested: "קופסה אפורה שקופה")
	// AHUD::DrawRect - K2_DrawRect not available on UCanvas in UE 5.4
	DrawRect(FLinearColor(0.5f, 0.5f, 0.5f, 0.35f), Min.X, Min.Y, W, H);
}
