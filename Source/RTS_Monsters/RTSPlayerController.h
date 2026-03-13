#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RTSDataTypes.h"
#include "RTSUnitInfoWidget.h"
#include "RTSPlayerController.generated.h"

class ARTSUnitCharacter;
class ARTSHeroCharacter;

/**
 * P1: Selection (LMB / Shift+LMB), box select (LMB drag), context order (RMB), command-radius authority.
 * Selection owned here (GDD); orders persist on units when leaving range – only new orders require authority (GDD).
 */
UCLASS(config = Game, meta = (DisplayName = "RTS Player Controller"))
class RTS_MONSTERS_API ARTSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ARTSPlayerController();

	// ---------- Selection API (Epic 1) ----------
	UFUNCTION(BlueprintCallable, Category = "RTS|Selection")
	void SetSelection(ARTSUnitCharacter* Unit);

	UFUNCTION(BlueprintCallable, Category = "RTS|Selection")
	void AddToSelection(ARTSUnitCharacter* Unit);

	UFUNCTION(BlueprintCallable, Category = "RTS|Selection")
	void RemoveFromSelection(ARTSUnitCharacter* Unit);

	UFUNCTION(BlueprintCallable, Category = "RTS|Selection")
	void ClearSelection();

	UFUNCTION(BlueprintPure, Category = "RTS|Selection")
	TArray<ARTSUnitCharacter*> GetSelection() const;

	/** Trace from cursor; returns hit unit if any, and hit location. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Input")
	bool GetHitUnderCursor(FVector& OutLocation, ARTSUnitCharacter*& OutUnit, AActor*& OutActor);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

private:
	/** LMB pressed: store position for possible box select. */
	void OnInputSelectPressed();
	/** LMB released: single click → select unit; drag → box select (GDD: LMB select, Shift+LMB add). */
	void OnInputSelectReleased();
	void OnInputOrderContext();
	void OnZoomIn();
	void OnZoomOut();

	/** P2 Phase 5: Secure Region – key S when Hero selected. */
	void OnInputSecureRegion();

	/** P2 Phase 6: Win/Lose feedback. */
	UFUNCTION()
	void OnGameWon(EFactionId Faction);
	UFUNCTION()
	void OnGameLost(EFactionId Faction);

	/** Pick issuer from selection (Hero preferred, else first unit with command authority). */
	ARTSUnitCharacter* GetOrderIssuer() const;

	/** P1 box select: get units whose screen position is inside the 2D box. Shift = add to selection. */
	void ApplyBoxSelect(const FVector2D& ScreenMin, const FVector2D& ScreenMax, bool bAddToSelection);

	/** Selected units (selection ownership; GDD). */
	UPROPERTY()
	TArray<TWeakObjectPtr<ARTSUnitCharacter>> SelectedUnits;

	/** Box select: LMB press screen position (for drag detection). */
	FVector2D BoxSelectPressScreenPos;
	bool bBoxSelectActive = false;

	/** True while middle mouse held for camera pitch (enables mouse capture so delta is reported). */
	bool bMiddleMouseHeldForPitch = false;

	/** Widget class for the unit info panel (red, right side). Set via Blueprint (BP_RTSPlayerController) or use UnitInfoWidgetPath in DefaultGame.ini. */
	UPROPERTY(EditDefaultsOnly, Category = "RTS|UI", meta = (AllowAbstract = "false"))
	TSubclassOf<URTSUnitInfoWidget> UnitInfoWidgetClass;

	/** If UnitInfoWidgetClass is not set, load widget from this path (e.g. /Game/RTS/WBP_UnitInfo.WBP_UnitInfo_C). Set in DefaultGame.ini [RTS_Monsters.RTSPlayerController]. */
	UPROPERTY(Config, VisibleAnywhere, Category = "RTS|UI")
	FString UnitInfoWidgetPath;
};
