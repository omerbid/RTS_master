#include "RTSSelectionBoxWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateColor.h"

void URTSSelectionBoxWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!WidgetTree) return;

	RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("SelectionRoot"));
	if (!RootCanvas)
	{
		return;
	}
	WidgetTree->RootWidget = RootCanvas;

	BoxBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("SelectionBox"));
	if (!BoxBorder)
	{
		return;
	}

	FLinearColor BoxColor(0.4f, 0.6f, 1.f, 0.25f);
	FSlateBrush Brush;
	Brush.TintColor = FSlateColor(BoxColor);
	Brush.DrawAs = ESlateBrushDrawType::Box;
	Brush.Margin = FMargin(0.f);
	BoxBorder->SetBrush(Brush);
	BoxBorder->SetVisibility(ESlateVisibility::Collapsed);

	BoxSlot = RootCanvas->AddChildToCanvas(BoxBorder);
	if (BoxSlot)
	{
		BoxSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
		BoxSlot->SetOffsets(FMargin(0.f, 0.f, 0.f, 0.f));
		BoxSlot->SetPosition(FVector2D(0.f, 0.f));
		BoxSlot->SetSize(FVector2D(0.f, 0.f));
	}
}

void URTSSelectionBoxWidget::UpdateBox(const FVector2D& ScreenMin, const FVector2D& ScreenMax)
{
	if (!BoxBorder || !BoxSlot)
	{
		return;
	}

	const float W = FMath::Max(1.f, ScreenMax.X - ScreenMin.X);
	const float H = FMath::Max(1.f, ScreenMax.Y - ScreenMin.Y);

	BoxSlot->SetPosition(ScreenMin);
	BoxSlot->SetSize(FVector2D(W, H));
	BoxBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void URTSSelectionBoxWidget::HideBox()
{
	if (BoxBorder)
	{
		BoxBorder->SetVisibility(ESlateVisibility::Collapsed);
	}
}
