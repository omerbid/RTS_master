#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RTSSelectionBoxWidget.generated.h"

class UBorder;
class UCanvasPanel;
class UCanvasPanelSlot;

/** Gray semi-transparent box shown when dragging to select units. */
UCLASS()
class RTS_MONSTERS_API URTSSelectionBoxWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Show box from ScreenMin to ScreenMax (viewport pixels). */
	void UpdateBox(const FVector2D& ScreenMin, const FVector2D& ScreenMax);
	void HideBox();

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY()
	TObjectPtr<UCanvasPanel> RootCanvas;

	UPROPERTY()
	TObjectPtr<UBorder> BoxBorder;

	UPROPERTY()
	TObjectPtr<UCanvasPanelSlot> BoxSlot;
};
