#include "RTSPlayerController.h"

#include "RTSCommandAuthorityComponent.h"
#include "RTSHeroCharacter.h"
#include "RTSUnitCharacter.h"
#include "RTSUnitInfoWidget.h"
#include "RTSCameraPawn.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "CollisionQueryParams.h"
#include "InputCoreTypes.h"
#include "Components/Widget.h"

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
}

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

	// Epic 2: LMB = select, Shift+LMB = add select, RMB = context order.
	InputComponent->BindKey(EKeys::LeftMouseButton, EInputEvent::IE_Pressed, this, &ARTSPlayerController::OnInputSelect);
	InputComponent->BindKey(EKeys::RightMouseButton, EInputEvent::IE_Pressed, this, &ARTSPlayerController::OnInputOrderContext);

	// Zoom: MouseScrollUp / MouseScrollDown (UE 5.4+; was MouseWheelUp/MouseWheelDown in older).
	InputComponent->BindKey(EKeys::MouseScrollUp, EInputEvent::IE_Pressed, this, &ARTSPlayerController::OnZoomIn);
	InputComponent->BindKey(EKeys::MouseScrollDown, EInputEvent::IE_Pressed, this, &ARTSPlayerController::OnZoomOut);

	// Pitch: MiddleMouse held + drag (PlayerTick).
}

void ARTSPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// Command range indicator: draw circle when Hero/Captain selected.
	UWorld* World = GetWorld();
	if (World)
	{
		ARTSUnitCharacter* Issuer = GetOrderIssuer();
		URTSCommandAuthorityComponent* Authority = Issuer ? (Cast<ARTSHeroCharacter>(Issuer) ? Cast<ARTSHeroCharacter>(Issuer)->CommandAuthorityComponent : Issuer->CommandAuthorityComponent) : nullptr;
		if (Authority && Authority->bAuthorityEnabled && Authority->CommandRadius > 0.f)
		{
			const FVector Center = Issuer->GetActorLocation();
			DrawDebugCircle(World, Center, Authority->CommandRadius, 32, FColor::Cyan, false, 0.f, 0, 2.f);
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

void ARTSPlayerController::OnInputSelect()
{
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
	return nullptr;
}
