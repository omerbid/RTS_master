#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "RTSUnitInfoWidget.generated.h"

class ARTSUnitCharacter;

/**
 * Displays selected unit data in red on the right side of the screen.
 * Created and updated by RTSPlayerController.
 */
UCLASS()
class RTS_MONSTERS_API URTSUnitInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Call when selection changes; pass first selected unit or null to clear. */
	UFUNCTION(BlueprintCallable, Category = "RTS|UI")
	void RefreshFromUnit(ARTSUnitCharacter* Unit);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void BuildWidgetTree();
	/** When using a Blueprint child, bind TextBlock_* from widgets named in the layout. */
	void BindWidgetsFromBlueprint();
	UTextBlock* AddRedTextBlock(const FString& Label);
	void SetTextBlock(UTextBlock* TB, const FText& Text);

	UPROPERTY()
	TObjectPtr<UTextBlock> TextBlock_UnitId;

	UPROPERTY()
	TObjectPtr<UTextBlock> TextBlock_Faction;

	UPROPERTY()
	TObjectPtr<UTextBlock> TextBlock_Morale;

	UPROPERTY()
	TObjectPtr<UTextBlock> TextBlock_Rank;

	UPROPERTY()
	TObjectPtr<UTextBlock> TextBlock_Extra;

	UPROPERTY()
	TObjectPtr<class UVerticalBox> VerticalBox_Root;

	TWeakObjectPtr<ARTSUnitCharacter> CachedUnit;
};
