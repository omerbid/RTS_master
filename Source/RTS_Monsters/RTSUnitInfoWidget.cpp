#include "RTSUnitInfoWidget.h"

#include "RTSPlayerController.h"
#include "RTSUnitCharacter.h"
#include "RTSMoraleComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

static const FLinearColor PanelColorRed(1.f, 0.2f, 0.2f, 1.f);

void URTSUnitInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (WidgetTree && WidgetTree->RootWidget)
	{
		BindWidgetsFromBlueprint();
	}
	else
	{
		BuildWidgetTree();
	}
	RefreshFromUnit(nullptr);
}

void URTSUnitInfoWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	ARTSPlayerController* RTSPC = Cast<ARTSPlayerController>(GetOwningPlayer());
	if (!RTSPC)
	{
		return;
	}
	TArray<ARTSUnitCharacter*> Sel = RTSPC->GetSelection();
	ARTSUnitCharacter* First = Sel.Num() > 0 ? Sel[0] : nullptr;
	if (First != CachedUnit.Get())
	{
		RefreshFromUnit(First);
	}
	else if (First)
	{
		RefreshFromUnit(First);
	}
}

void URTSUnitInfoWidget::RefreshFromUnit(ARTSUnitCharacter* Unit)
{
	CachedUnit = Unit;
	if (!TextBlock_UnitId || !TextBlock_Faction || !TextBlock_Morale || !TextBlock_Rank || !TextBlock_Extra)
	{
		return;
	}
	if (!Unit)
	{
		SetTextBlock(TextBlock_UnitId, FText::FromString(TEXT("-- No selection --")));
		SetTextBlock(TextBlock_Faction, FText());
		SetTextBlock(TextBlock_Morale, FText());
		SetTextBlock(TextBlock_Rank, FText());
		SetTextBlock(TextBlock_Extra, FText());
		return;
	}
	SetTextBlock(TextBlock_UnitId, FText::FromName(Unit->UnitId));
	FString FactionStr;
	switch (Unit->FactionId)
	{
		case EFactionId::Humans:     FactionStr = TEXT("Humans"); break;
		case EFactionId::Vampires:   FactionStr = TEXT("Vampires"); break;
		case EFactionId::Werewolves: FactionStr = TEXT("Werewolves"); break;
		default: FactionStr = TEXT("?"); break;
	}
	SetTextBlock(TextBlock_Faction, FText::FromString(FactionStr));
	float Morale = Unit->MoraleComponent ? Unit->MoraleComponent->CurrentMorale : 0.f;
	SetTextBlock(TextBlock_Morale, FText::FromString(FString::Printf(TEXT("Morale: %.0f"), Morale)));
	SetTextBlock(TextBlock_Rank, FText::FromString(FString::Printf(TEXT("Rank: %d"), Unit->Rank)));
	FString Extra;
	if (Unit->bIsCaptain)
	{
		Extra += TEXT("Captain ");
	}
	if (Unit->bIsDetached)
	{
		Extra += TEXT("Detached");
	}
	SetTextBlock(TextBlock_Extra, FText::FromString(Extra.IsEmpty() ? TEXT("--") : Extra));
}

void URTSUnitInfoWidget::BuildWidgetTree()
{
	// Pure C++ widgets can have null WidgetTree in some contexts; if the panel never appears,
	// use a Blueprint child of this class and build the layout there.
	if (!WidgetTree)
	{
		return;
	}
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
	if (!Root)
	{
		return;
	}
	WidgetTree->RootWidget = Root;

	VerticalBox_Root = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	if (!VerticalBox_Root)
	{
		return;
	}
	Root->AddChild(VerticalBox_Root);
	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(VerticalBox_Root->Slot);
	if (CanvasSlot)
	{
		CanvasSlot->SetAnchors(FAnchors(1.f, 0.f, 1.f, 0.f));
		CanvasSlot->SetAlignment(FVector2D(1.f, 0.f));
		CanvasSlot->SetPosition(FVector2D(-220.f, 20.f));
		CanvasSlot->SetSize(FVector2D(200.f, 280.f));
	}

	TextBlock_UnitId = AddRedTextBlock(TEXT("--"));
	TextBlock_Faction = AddRedTextBlock(TEXT(""));
	TextBlock_Morale = AddRedTextBlock(TEXT("Morale: --"));
	TextBlock_Rank = AddRedTextBlock(TEXT("Rank: --"));
	TextBlock_Extra = AddRedTextBlock(TEXT("--"));
}

void URTSUnitInfoWidget::BindWidgetsFromBlueprint()
{
	if (!WidgetTree) return;
	auto FindText = [this](const TCHAR* Preferred, const TCHAR* Fallback) -> UTextBlock*
	{
		UWidget* W = WidgetTree->FindWidget(FName(Preferred));
		if (!W) W = WidgetTree->FindWidget(FName(Fallback));
		return Cast<UTextBlock>(W);
	};
	TextBlock_UnitId   = FindText(TEXT("TextBlock_UnitId"), TEXT("UnitId"));
	TextBlock_Faction  = FindText(TEXT("TextBlock_Faction"), TEXT("Faction"));
	TextBlock_Morale   = FindText(TEXT("TextBlock_Morale"), TEXT("Morale"));
	TextBlock_Rank     = FindText(TEXT("TextBlock_Rank"), TEXT("Rank"));
	TextBlock_Extra    = FindText(TEXT("TextBlock_Extra"), TEXT("Extra"));
}

UTextBlock* URTSUnitInfoWidget::AddRedTextBlock(const FString& Label)
{
	if (!WidgetTree || !VerticalBox_Root)
	{
		return nullptr;
	}
	UClass* TextBlockClass = UTextBlock::StaticClass();
	UTextBlock* TB = WidgetTree->ConstructWidget<UTextBlock>(TextBlockClass);
	if (TB)
	{
		TB->SetColorAndOpacity(FSlateColor(PanelColorRed));
		TB->SetText(FText::FromString(Label));
		VerticalBox_Root->AddChild(TB);
	}
	return TB;
}

void URTSUnitInfoWidget::SetTextBlock(UTextBlock* TB, const FText& Text)
{
	if (TB)
	{
		TB->SetText(Text);
		TB->SetColorAndOpacity(FSlateColor(PanelColorRed));
	}
}
