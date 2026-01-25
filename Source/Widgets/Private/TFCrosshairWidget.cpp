// Copyright TF Project. All Rights Reserved.

#include "TFCrosshairWidget.h"
#include "TFPlayerCharacter.h"
#include "TFInteractionComponent.h"
#include "TFInteractableInterface.h"
#include "TFPlayerController.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"

void UTFCrosshairWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize player character
	InitializePlayerCharacter();

	// Initialize current values
	CurrentSize = DefaultSize;
	TargetSize = DefaultSize;
	CurrentColor = DefaultColor;
	TargetColor = DefaultColor;

	// Get canvas slot for positioning
	if (CrosshairImage && CrosshairCanvas)
	{
		CrosshairSlot = Cast<UCanvasPanelSlot>(CrosshairImage->Slot);
	}

	// Set initial size
	if (CrosshairImage)
	{
		CrosshairImage->SetDesiredSizeOverride(CurrentSize);
	}

	// Initialize position to screen center
	if (GEngine && GEngine->GameViewport)
	{
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		CurrentScreenPosition = ViewportSize * 0.5f;
		TargetScreenPosition = CurrentScreenPosition;
	}

	// Ensure initial visibility (SelfHitTestInvisible so it doesn't block input)
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UTFCrosshairWidget::NativeDestruct()
{
	CachedPlayerCharacter.Reset();

	Super::NativeDestruct();
}

void UTFCrosshairWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update visibility
	UpdateVisibility();

	if (GetVisibility() == ESlateVisibility::Hidden)
	{
		return;
	}

	// Ensure we have a valid player character
	if (!CachedPlayerCharacter.IsValid())
	{
		InitializePlayerCharacter();
		if (!CachedPlayerCharacter.IsValid())
		{
			return;
		}
	}

	// Perform trace
	FHitResult HitResult;
	bHasHit = PerformTrace(HitResult);

	if (bHasHit)
	{
		CurrentHitLocation = HitResult.ImpactPoint;
		UpdateCrosshairPosition(HitResult, InDeltaTime);
		UpdateCrosshairVisuals(HitResult, InDeltaTime);
	}
	else
	{
		UpdateCrosshairPositionNoHit(InDeltaTime);

		// Reset to default visuals
		bIsAimingAtInteractable = false;
		TargetColor = DefaultColor;
		TargetSize = DefaultSize;
	}

	// Interpolate and apply properties
	InterpolateCrosshairProperties(InDeltaTime);
	ApplyCrosshairProperties();
}

void UTFCrosshairWidget::InitializePlayerCharacter()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerPawn)
	{
		CachedPlayerCharacter = Cast<ATFPlayerCharacter>(PlayerPawn);
	}
}

bool UTFCrosshairWidget::PerformTrace(FHitResult& OutHitResult)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FVector TraceStart, TraceEnd;
	if (!GetTracePoints(TraceStart, TraceEnd))
	{
		return false;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(TFCrosshairTrace), bTraceComplex);

	if (CachedPlayerCharacter.IsValid())
	{
		QueryParams.AddIgnoredActor(CachedPlayerCharacter.Get());
	}

	return World->LineTraceSingleByChannel(
		OutHitResult,
		TraceStart,
		TraceEnd,
		TraceChannel,
		QueryParams
	);
}

bool UTFCrosshairWidget::GetTracePoints(FVector& TraceStart, FVector& TraceEnd) const
{
	if (!CachedPlayerCharacter.IsValid())
	{
		return false;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		return false;
	}

	// Get camera location and rotation from player controller
	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	TraceStart = CameraLocation;
	TraceEnd = TraceStart + (CameraRotation.Vector() * TraceDistance);

	return true;
}

void UTFCrosshairWidget::UpdateCrosshairPosition(const FHitResult& HitResult, float DeltaTime)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		return;
	}

	FVector2D ScreenPosition;
	if (PC->ProjectWorldLocationToScreen(HitResult.ImpactPoint, ScreenPosition, true))
	{
		TargetScreenPosition = ScreenPosition;
	}
}

void UTFCrosshairWidget::UpdateCrosshairPositionNoHit(float DeltaTime)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		return;
	}

	// When no hit, project the end point of the trace to screen
	FVector TraceStart, TraceEnd;
	if (GetTracePoints(TraceStart, TraceEnd))
	{
		FVector2D ScreenPosition;
		if (PC->ProjectWorldLocationToScreen(TraceEnd, ScreenPosition, true))
		{
			TargetScreenPosition = ScreenPosition;
		}
	}
}

void UTFCrosshairWidget::UpdateCrosshairVisuals(const FHitResult& HitResult, float DeltaTime)
{
	AActor* HitActor = HitResult.GetActor();

	if (IsInteractable(HitActor))
	{
		bIsAimingAtInteractable = true;

		if (CanInteract(HitActor))
		{
			TargetColor = InteractableColor;
		}
		else
		{
			TargetColor = CannotInteractColor;
		}

		TargetSize = InteractableSize;
	}
	else
	{
		bIsAimingAtInteractable = false;
		TargetColor = DefaultColor;
		TargetSize = DefaultSize;
	}
}

void UTFCrosshairWidget::InterpolateCrosshairProperties(float DeltaTime)
{
	// Interpolate position
	if (PositionSmoothSpeed > 0.0f)
	{
		CurrentScreenPosition = FMath::Vector2DInterpTo(
			CurrentScreenPosition,
			TargetScreenPosition,
			DeltaTime,
			PositionSmoothSpeed
		);
	}
	else
	{
		CurrentScreenPosition = TargetScreenPosition;
	}

	// Interpolate size
	CurrentSize = FMath::Vector2DInterpTo(
		CurrentSize,
		TargetSize,
		DeltaTime,
		SizeInterpSpeed
	);

	// Interpolate color
	CurrentColor = FMath::CInterpTo(
		CurrentColor,
		TargetColor,
		DeltaTime,
		ColorInterpSpeed
	);
}

void UTFCrosshairWidget::ApplyCrosshairProperties()
{
	if (!CrosshairImage || !CrosshairSlot)
	{
		return;
	}

	// Apply position (offset by half size to center the crosshair)
	FVector2D CenteredPosition = CurrentScreenPosition - (CurrentSize * 0.5f);
	CrosshairSlot->SetPosition(CenteredPosition);

	// Apply size
	CrosshairImage->SetDesiredSizeOverride(CurrentSize);

	// Apply color
	CrosshairImage->SetColorAndOpacity(CurrentColor);
}

bool UTFCrosshairWidget::IsInteractable(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	return Actor->Implements<UTFInteractableInterface>();
}

bool UTFCrosshairWidget::CanInteract(AActor* Actor) const
{
	if (!Actor || !CachedPlayerCharacter.IsValid())
	{
		return false;
	}

	ITFInteractableInterface* Interactable = Cast<ITFInteractableInterface>(Actor);
	if (!Interactable)
	{
		return false;
	}

	return Interactable->CanInteract(CachedPlayerCharacter.Get());
}

void UTFCrosshairWidget::UpdateVisibility()
{
	if (!bHideWhenUIOpen)
	{
		// Always ensure visible if we're not managing visibility
		if (GetVisibility() != ESlateVisibility::SelfHitTestInvisible)
		{
			SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	ATFPlayerController* TFPC = Cast<ATFPlayerController>(PC);

	// Default to showing crosshair unless UI is explicitly blocking
	bool bShouldHide = false;

	if (TFPC)
	{
		bShouldHide = TFPC->IsUIBlockingInput();
	}

	if (bShouldHide)
	{
		if (GetVisibility() != ESlateVisibility::Hidden)
		{
			SetVisibility(ESlateVisibility::Hidden);
		}
	}
	else
	{
		// Use SelfHitTestInvisible so crosshair doesn't block mouse input
		if (GetVisibility() != ESlateVisibility::SelfHitTestInvisible)
		{
			SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
}
