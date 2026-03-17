#include "RTSPlayerController.h"

#include "RTSCommandAuthorityComponent.h"
#include "RTSDayNightSubsystem.h"
#include "RTSHeroCharacter.h"
#include "RTSUnitCharacter.h"
#include "RTSUnitInfoWidget.h"
#include "RTSVictorySubsystem.h"
#include "RTSEconomySubsystem.h"
#include "RTSDataRegistry.h"
#include "RTSRegionVolume.h"
#include "RTSCameraPawn.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "CollisionQueryParams.h"
#include "InputCoreTypes.h"
#include "Components/Widget.h"
#include "Kismet/GameplayStatics.h"

static const float TraceDistance = 50000.f;

ARTSPlayerController::ARTSPlayerController()
{
}

void ARTSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// RTS: show mouse cursor and allow click events so we can select units.
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	// Input mode: cursor visible, no mouse capture – required for RTS selection.
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	// Unit info panel (red, right side). Note: creating UUserWidget from C++ can trigger
	// a crash when using Live Coding (patch DLL static init in UMG). Use full build without Live Coding.
	UClass* WidgetClass = UnitInfoWidgetClass.Get();
	if (!WidgetClass && !UnitInfoWidgetPath.IsEmpty())
	{
		WidgetClass = LoadObject<UClass>(nullptr, *UnitInfoWidgetPath);
	}
	if (!WidgetClass)
	{
		WidgetClass = URTSUnitInfoWidget::StaticClass();
	}
	if (URTSUnitInfoWidget* UnitInfo = CreateWidget<URTSUnitInfoWidget>(this, WidgetClass))
	{
		UnitInfo->AddToViewport(10);
		UnitInfo->SetVisibility(ESlateVisibility::Visible);
		UnitInfo->RefreshFromUnit(nullptr);
	}

	// P2 Phase 6: Subscribe to Win/Lose for feedback.
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (URTSVictorySubsystem* Victory = GI->GetSubsystem<URTSVictorySubsystem>())
		{
			Victory->OnGameWon.AddDynamic(this, &ARTSPlayerController::OnGameWon);
			Victory->OnGameLost.AddDynamic(this, &ARTSPlayerController::OnGameLost);
		}
		// P6: Day/Night phase change feedback.
		if (URTSDayNightSubsystem* DayNight = GI->GetSubsystem<URTSDayNightSubsystem>())
		{
			DayNight->OnDayNightPhaseChanged.AddDynamic(this, &ARTSPlayerController::OnDayNightPhaseChanged);
			// Show current phase once at start.
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.f, FColor::Cyan,
					DayNight->IsNight() ? TEXT("[Day/Night] Night") : TEXT("[Day/Night] Day"));
			}
		}
	}
}

// GDD: Selection is owned by PlayerController; only units in Hero/Captain command range receive new orders; orders persist when leaving range.
void ARTSPlayerController::SetSelection(ARTSUnitCharacter* Unit)
{
	for (const TWeakObjectPtr<ARTSUnitCharacter>& Ptr : SelectedUnits)
	{
		if (ARTSUnitCharacter* U = Ptr.Get())
		{
			U->SetSelected(false);
		}
	}
	SelectedUnits.Reset();
	if (Unit)
	{
		SelectedUnits.Add(Unit);
		Unit->SetSelected(true);
	}
}

void ARTSPlayerController::AddToSelection(ARTSUnitCharacter* Unit)
{
	if (Unit && !SelectedUnits.Contains(Unit))
	{
		SelectedUnits.Add(Unit);
		Unit->SetSelected(true);
	}
}

void ARTSPlayerController::RemoveFromSelection(ARTSUnitCharacter* Unit)
{
	if (Unit)
	{
		Unit->SetSelected(false);
	}
	SelectedUnits.Remove(Unit);
}

void ARTSPlayerController::ClearSelection()
{
	for (const TWeakObjectPtr<ARTSUnitCharacter>& Ptr : SelectedUnits)
	{
		if (ARTSUnitCharacter* U = Ptr.Get())
		{
			U->SetSelected(false);
		}
	}
	SelectedUnits.Reset();
}

TArray<ARTSUnitCharacter*> ARTSPlayerController::GetSelection() const
{
	TArray<ARTSUnitCharacter*> Result;
	for (const TWeakObjectPtr<ARTSUnitCharacter>& Ptr : SelectedUnits)
	{
		if (ARTSUnitCharacter* U = Ptr.Get())
		{
			Result.Add(U);
		}
	}
	return Result;
}

bool ARTSPlayerController::GetHitUnderCursor(FVector& OutLocation, ARTSUnitCharacter*& OutUnit, AActor*& OutActor)
{
	OutUnit = nullptr;
	OutActor = nullptr;

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FVector WorldLoc, WorldDir;
	if (!DeprojectMousePositionToWorld(WorldLoc, WorldDir))
	{
		return false;
	}

	const FVector Start = WorldLoc;
	const FVector End = WorldLoc + WorldDir * TraceDistance;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(RTSTrace), true);

	auto FindUnitInHits = [&OutUnit, &OutActor, &OutLocation](const TArray<FHitResult>& InHits) -> bool
	{
		for (const FHitResult& Hit : InHits)
		{
			AActor* Actor = Hit.GetActor();
			if (ARTSUnitCharacter* U = Cast<ARTSUnitCharacter>(Actor))
			{
				OutUnit = U;
				OutActor = Actor;
				OutLocation = Hit.ImpactPoint;
				return true;
			}
		}
		return false;
	};

	TArray<FHitResult> Hits;
	World->LineTraceMultiByChannel(Hits, Start, End, ECC_Visibility, Params);
	if (FindUnitInHits(Hits))
	{
		return true;
	}

	// Fallback: ECC_Pawn (Character blocks Pawn; Blueprint may not block Visibility).
	TArray<FHitResult> PawnHits;
	World->LineTraceMultiByChannel(PawnHits, Start, End, ECC_Pawn, Params);
	if (FindUnitInHits(PawnHits))
	{
		return true;
	}

	// No unit: use first Visibility hit for ground (or End if none).
	if (Hits.Num() > 0)
	{
		OutLocation = Hits[0].ImpactPoint;
		OutActor = Hits[0].GetActor();
	}
	else
	{
		OutLocation = End;
	}
	return true;
}

void ARTSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!InputComponent)
	{
		return;
	}

	// P1: LMB pressed = store for box/drag; LMB released = single select or box select. RMB = context order.
	InputComponent->BindKey(EKeys::LeftMouseButton, EInputEvent::IE_Pressed, this, &ARTSPlayerController::OnInputSelectPressed);
	InputComponent->BindKey(EKeys::LeftMouseButton, EInputEvent::IE_Released, this, &ARTSPlayerController::OnInputSelectReleased);
	InputComponent->BindKey(EKeys::RightMouseButton, EInputEvent::IE_Pressed, this, &ARTSPlayerController::OnInputOrderContext);

	// Zoom: MouseScrollUp / MouseScrollDown (UE 5.4+; was MouseWheelUp/MouseWheelDown in older).
	InputComponent->BindKey(EKeys::MouseScrollUp, EInputEvent::IE_Pressed, this, &ARTSPlayerController::OnZoomIn);
	InputComponent->BindKey(EKeys::MouseScrollDown, EInputEvent::IE_Pressed, this, &ARTSPlayerController::OnZoomOut);

	// P2 Phase 5: Secure Region – key G when Hero selected (S conflicts with camera backward).
	InputComponent->BindKey(EKeys::G, EInputEvent::IE_Pressed, this, &ARTSPlayerController::OnInputSecureRegion);

	// P4: Recruit – key R when Hero selected.
	InputComponent->BindKey(EKeys::R, EInputEvent::IE_Pressed, this, &ARTSPlayerController::OnInputRecruit);

	// P5: Perform Ritual – key B when respawn available (after Hero death, 90s timer ended).
	InputComponent->BindKey(EKeys::B, EInputEvent::IE_Pressed, this, &ARTSPlayerController::OnInputPerformRitual);

	// Pitch: MiddleMouse held + drag (PlayerTick).
}

void ARTSPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// Command range indicator: draw circle when Hero/Captain selected (skip when game over).
	UWorld* World = GetWorld();
	if (World)
	{
		URTSVictorySubsystem* Victory = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<URTSVictorySubsystem>() : nullptr;
		if (!Victory || !Victory->IsGameOver())
		{
			ARTSUnitCharacter* Issuer = GetOrderIssuer();
			URTSCommandAuthorityComponent* Authority = Issuer ? (Cast<ARTSHeroCharacter>(Issuer) ? Cast<ARTSHeroCharacter>(Issuer)->CommandAuthorityComponent : Issuer->CommandAuthorityComponent) : nullptr;
			if (Authority && Authority->bAuthorityEnabled && Authority->CommandRadius > 0.f)
			{
				const FVector Center = Issuer->GetActorLocation();
				DrawDebugCircle(World, Center, Authority->CommandRadius, 32, FColor::Cyan, false, 0.f, 0, 2.f);
			}
		}
	}

	// Pitch: MiddleMouse held + mouse Y. Capture mouse while held so GetInputMouseDelta works.
	if (IsInputKeyDown(EKeys::MiddleMouseButton))
	{
		if (!bMiddleMouseHeldForPitch)
		{
			bMiddleMouseHeldForPitch = true;
			FInputModeGameOnly InputMode;
			SetInputMode(InputMode);
			bShowMouseCursor = false;
			int32 SizeX = 0, SizeY = 0;
			GetViewportSize(SizeX, SizeY);
			SetMouseLocation(SizeX / 2, SizeY / 2);
		}
		float DeltaX = 0.f, DeltaY = 0.f;
		GetInputMouseDelta(DeltaX, DeltaY);
		if (!FMath::IsNearlyZero(DeltaY) && GetPawn())
		{
			if (ARTSCameraPawn* Cam = Cast<ARTSCameraPawn>(GetPawn()))
			{
				Cam->AdjustCameraPitch(DeltaY);
			}
		}
	}
	else if (bMiddleMouseHeldForPitch)
	{
		bMiddleMouseHeldForPitch = false;
		bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		SetInputMode(InputMode);
	}
}

void ARTSPlayerController::OnZoomIn()
{
	if (ARTSCameraPawn* Cam = Cast<ARTSCameraPawn>(GetPawn()))
	{
		Cam->ZoomInOut(1.f);
	}
}

void ARTSPlayerController::OnZoomOut()
{
	if (ARTSCameraPawn* Cam = Cast<ARTSCameraPawn>(GetPawn()))
	{
		Cam->ZoomInOut(-1.f);
	}
}

void ARTSPlayerController::OnInputSelectPressed()
{
	float MouseX = 0.f, MouseY = 0.f;
	if (GetMousePosition(MouseX, MouseY))
	{
		BoxSelectPressScreenPos = FVector2D(MouseX, MouseY);
		bBoxSelectActive = true;
	}
}

void ARTSPlayerController::OnInputSelectReleased()
{
	if (!bBoxSelectActive) return;
	bBoxSelectActive = false;

	float MouseX = 0.f, MouseY = 0.f;
	if (!GetMousePosition(MouseX, MouseY))
	{
		return;
	}
	FVector2D ReleasePos(MouseX, MouseY);
	const float DragThresholdPx = 5.f;
	const float DragDist = FVector2D::Distance(BoxSelectPressScreenPos, ReleasePos);

	if (DragDist >= DragThresholdPx)
	{
		// Box select: min/max screen rect
		FVector2D ScreenMin(FMath::Min(BoxSelectPressScreenPos.X, ReleasePos.X), FMath::Min(BoxSelectPressScreenPos.Y, ReleasePos.Y));
		FVector2D ScreenMax(FMath::Max(BoxSelectPressScreenPos.X, ReleasePos.X), FMath::Max(BoxSelectPressScreenPos.Y, ReleasePos.Y));
		const bool bAdd = IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);
		ApplyBoxSelect(ScreenMin, ScreenMax, bAdd);
		return;
	}

	// Single click: same behavior as before (select unit under cursor or clear).
	FVector Loc;
	ARTSUnitCharacter* Unit = nullptr;
	AActor* Actor = nullptr;
	if (!GetHitUnderCursor(Loc, Unit, Actor))
	{
		return;
	}
	if (Unit)
	{
		if (IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift))
		{
			AddToSelection(Unit);
		}
		else
		{
			SetSelection(Unit);
		}
	}
	else
	{
		ClearSelection();
	}
}

void ARTSPlayerController::ApplyBoxSelect(const FVector2D& ScreenMin, const FVector2D& ScreenMax, bool bAddToSelection)
{
	UWorld* World = GetWorld();
	if (!World) return;

	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(World, ARTSUnitCharacter::StaticClass(), Found);

	TArray<ARTSUnitCharacter*> InBox;
	for (AActor* A : Found)
	{
		ARTSUnitCharacter* U = Cast<ARTSUnitCharacter>(A);
		if (!U || !IsValid(U)) continue;

		FVector2D ScreenPos;
		if (ProjectWorldLocationToScreen(U->GetActorLocation(), ScreenPos))
		{
			if (ScreenPos.X >= ScreenMin.X && ScreenPos.X <= ScreenMax.X && ScreenPos.Y >= ScreenMin.Y && ScreenPos.Y <= ScreenMax.Y)
			{
				InBox.Add(U);
			}
		}
	}

	if (InBox.Num() == 0) return;

	if (!bAddToSelection)
	{
		ClearSelection();
	}
	for (ARTSUnitCharacter* U : InBox)
	{
		AddToSelection(U);
	}
}

void ARTSPlayerController::OnInputOrderContext()
{
	FVector HitLoc;
	ARTSUnitCharacter* HitUnit = nullptr;
	AActor* HitActor = nullptr;
	if (!GetHitUnderCursor(HitLoc, HitUnit, HitActor))
	{
		return;
	}

	ARTSUnitCharacter* Issuer = GetOrderIssuer();
	if (!Issuer)
	{
		return;
	}

	// Shift+RMB on selected unit of same faction: Promote to Captain (P3).
	const bool bShiftHeld = IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);
	if (bShiftHeld && HitUnit && HitUnit != Issuer && HitUnit->FactionId == Issuer->FactionId)
	{
		TArray<ARTSUnitCharacter*> Selection = GetSelection();
		if (Selection.Contains(HitUnit) && HitUnit->TryPromoteToCaptain(Issuer))
		{
			return; // Promoted; no move order.
		}
	}

	ARTSHeroCharacter* Hero = Cast<ARTSHeroCharacter>(Issuer);
	URTSCommandAuthorityComponent* Authority = Hero ? Hero->CommandAuthorityComponent : Issuer->CommandAuthorityComponent;
	if (!Authority)
	{
		return;
	}

	TArray<ARTSUnitCharacter*> Selection = GetSelection();
	for (ARTSUnitCharacter* Unit : Selection)
	{
		if (!Unit || !IsValid(Unit))
		{
			continue;
		}
		// GDD: new orders require unit to be in issuer command radius; existing orders persist when unit leaves range.
		if (!Authority->CanIssueOrderToUnit(Unit))
		{
			continue;
		}

		if (HitUnit && HitUnit != Unit)
		{
			// Enemy or other actor: Attack if different faction.
			if (HitUnit->FactionId != Issuer->FactionId)
			{
				FRTSOrderPayload Payload;
				Payload.AttackTarget = HitUnit;
				Unit->SetCurrentOrder(ERTSOrderType::Attack, Payload);
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(INDEX_NONE, 2.f, FColor::Orange, TEXT("[RTS] Attack order issued (RMB on enemy)"));
				}
			}
			else
			{
				// Same faction: move to unit location
				FRTSOrderPayload Payload;
				Payload.MoveDestination = HitUnit->GetActorLocation();
				Unit->SetCurrentOrder(ERTSOrderType::Move, Payload);
			}
		}
		else
		{
			// Ground or no unit: Move
			FRTSOrderPayload Payload;
			Payload.MoveDestination = HitLoc;
			Unit->SetCurrentOrder(ERTSOrderType::Move, Payload);
		}
	}
}

void ARTSPlayerController::OnInputRecruit()
{
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (URTSVictorySubsystem* Victory = GI->GetSubsystem<URTSVictorySubsystem>())
		{
			if (Victory->IsGameOver()) return;
		}
	}
	ARTSUnitCharacter* Issuer = GetOrderIssuer();
	ARTSHeroCharacter* Hero = Issuer ? Cast<ARTSHeroCharacter>(Issuer) : nullptr;
	if (!Hero)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 2.f, FColor::Yellow, TEXT("[RTS] Select a Hero to recruit (key R)"));
		return;
	}
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI) return;
	URTSDataRegistry* Registry = GI->GetSubsystem<URTSDataRegistry>();
	URTSEconomySubsystem* Economy = GI->GetSubsystem<URTSEconomySubsystem>();
	if (!Registry || !Economy)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 2.f, FColor::Red, TEXT("[RTS] Economy system not available"));
		return;
	}
	FName UnitId = Registry->GetFirstRecruitableUnitIdForFaction(Hero->FactionId);
	if (UnitId == NAME_None)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 2.f, FColor::Orange, TEXT("[RTS] No recruitable unit for this faction"));
		return;
	}
	if (ARTSUnitCharacter* NewUnit = Economy->TryRecruitUnit(Hero, UnitId))
	{
		AddToSelection(NewUnit);  // Add to selection so it can receive move orders
		if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, FColor::Green, FString::Printf(TEXT("[RTS] Unit recruited: %s"), *UnitId.ToString()));
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 2.f, FColor::Orange, TEXT("[RTS] Cannot recruit: not enough resources or Hero not in region"));
	}
}

void ARTSPlayerController::OnInputSecureRegion()
{
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (URTSVictorySubsystem* Victory = GI->GetSubsystem<URTSVictorySubsystem>())
		{
			if (Victory->IsGameOver()) return;
		}
	}
	ARTSUnitCharacter* Issuer = GetOrderIssuer();
	ARTSHeroCharacter* Hero = Issuer ? Cast<ARTSHeroCharacter>(Issuer) : nullptr;
	if (!Hero)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 2.f, FColor::Yellow, TEXT("[RTS] Select a Hero to Secure Region (key G)"));
		return;
	}
	if (Hero->TryStartSecureRegion())
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, FColor::Green, TEXT("[RTS] Secure Region started – hold position 15s"));
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 2.f, FColor::Orange, TEXT("[RTS] Cannot Secure: Hero must be in region, control 4, not contested"));
	}
}

void ARTSPlayerController::OnInputPerformRitual()
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI) return;
	URTSVictorySubsystem* Victory = GI->GetSubsystem<URTSVictorySubsystem>();
	if (!Victory || Victory->IsGameOver()) return;

	EFactionId Faction;
	if (!Victory->GetFactionWithRitualAvailable(Faction))
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 2.f, FColor::Yellow, TEXT("[RTS] No respawn ritual available (key B). Wait for timer after Hero death."));
		return;
	}
	if (Victory->StartRitualChannel(Faction))
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 4.f, FColor::Cyan, TEXT("[RTS] Ritual started – 10s to respawn Hero"));
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 2.f, FColor::Orange, TEXT("[RTS] Cannot start ritual"));
	}
}

void ARTSPlayerController::OnGameWon(EFactionId Faction)
{
	if (GEngine)
	{
		FText FactionName = Faction == EFactionId::Humans ? FText::FromString(TEXT("Humans")) :
			Faction == EFactionId::Vampires ? FText::FromString(TEXT("Vampires")) : FText::FromString(TEXT("Werewolves"));
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Green, FString::Printf(TEXT("*** VICTORY! %s ***"), *FactionName.ToString()));
	}
	SetPause(true);
}

void ARTSPlayerController::OnGameLost(EFactionId Faction)
{
	if (GEngine)
	{
		FText FactionName = Faction == EFactionId::Humans ? FText::FromString(TEXT("Humans")) :
			Faction == EFactionId::Vampires ? FText::FromString(TEXT("Vampires")) : FText::FromString(TEXT("Werewolves"));
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Red, FString::Printf(TEXT("*** DEFEAT! %s ***"), *FactionName.ToString()));
	}
	SetPause(true);
}

void ARTSPlayerController::OnDayNightPhaseChanged(ERTSDayNightPhase NewPhase)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 4.f, FColor::Cyan,
			NewPhase == ERTSDayNightPhase::Night ? TEXT("[Day/Night] Night falls") : TEXT("[Day/Night] Dawn"));
	}
}

ARTSUnitCharacter* ARTSPlayerController::GetOrderIssuer() const
{
	// P1: issuer = first Hero in selection. P3: else first Captain (unit with CommandAuthorityComponent).
	for (const TWeakObjectPtr<ARTSUnitCharacter>& Ptr : SelectedUnits)
	{
		ARTSUnitCharacter* U = Ptr.Get();
		if (!U)
		{
			continue;
		}
		ARTSHeroCharacter* Hero = Cast<ARTSHeroCharacter>(U);
		if (Hero && Hero->CommandAuthorityComponent)
		{
			return Hero;
		}
		if (U->CommandAuthorityComponent)
		{
			return U; // Captain
		}
	}

	// No Hero/Captain in selection: find Hero of first selected unit's faction (so player can order recruited units).
	ARTSUnitCharacter* FirstSelected = SelectedUnits.Num() > 0 ? SelectedUnits[0].Get() : nullptr;
	if (FirstSelected && GetWorld())
	{
		for (TActorIterator<ARTSHeroCharacter> It(GetWorld()); It; ++It)
		{
			ARTSHeroCharacter* Hero = *It;
			if (IsValid(Hero) && Hero->FactionId == FirstSelected->FactionId && Hero->CommandAuthorityComponent)
			{
				return Hero;
			}
		}
	}
	return nullptr;
}
