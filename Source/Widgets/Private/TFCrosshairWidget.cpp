// Copyright TF Project. All Rights Reserved.

#include "TFCrosshairWidget.h"
#include "TFPlayerCharacter.h"
#include "TFInteractionComponent.h"
#include "TFPlayerController.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SkeletalMeshComponent.h"
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
		if (CrosshairSlot)
		{
			// Set anchor to center and alignment to center for proper centering
			CrosshairSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
			CrosshairSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		}
	}

	// Set initial size
	if (CrosshairImage)
	{
		CrosshairImage->SetDesiredSizeOverride(CurrentSize);
	}

	// Initialize position to center (0,0 offset from anchor at center)
	CurrentScreenPosition = FVector2D::ZeroVector;
	TargetScreenPosition = FVector2D::ZeroVector;

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

	// Skip processing if crosshair is hidden
	if (CrosshairImage && CrosshairImage->GetVisibility() == ESlateVisibility::Hidden)
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
		// Convert to offset from screen center
		if (GEngine && GEngine->GameViewport)
		{
			FVector2D ViewportSize;
			GEngine->GameViewport->GetViewportSize(ViewportSize);
			FVector2D ScreenCenter = ViewportSize * 0.5f;

			// Calculate offset from center, then convert to Slate units
			FVector2D Offset = ScreenPosition - ScreenCenter;
			const float ViewportScale = GEngine->GameViewport->GetDPIScale();
			if (ViewportScale > 0.0f)
			{
				Offset /= ViewportScale;
			}
			TargetScreenPosition = Offset;
		}
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
			// Convert to offset from screen center
			if (GEngine && GEngine->GameViewport)
			{
				FVector2D ViewportSize;
				GEngine->GameViewport->GetViewportSize(ViewportSize);
				FVector2D ScreenCenter = ViewportSize * 0.5f;

				// Calculate offset from center, then convert to Slate units
				FVector2D Offset = ScreenPosition - ScreenCenter;
				const float ViewportScale = GEngine->GameViewport->GetDPIScale();
				if (ViewportScale > 0.0f)
				{
					Offset /= ViewportScale;
				}
				TargetScreenPosition = Offset;
			}
		}
	}
}

void UTFCrosshairWidget::UpdateCrosshairVisuals(const FHitResult& HitResult, float DeltaTime)
{
	// Query the InteractionComponent to determine if we can actually interact
	// This ensures the crosshair matches the real interaction system exactly
	if (CachedPlayerCharacter.IsValid())
	{
		UTFInteractionComponent* InteractionComp = CachedPlayerCharacter->GetInteractionComponent();
		if (InteractionComp && InteractionComp->HasInteractable())
		{
			AActor* HitActor = HitResult.GetActor();
			AActor* FocusedActor = InteractionComp->GetCurrentInteractable();

			// Only show green if the crosshair trace is hitting the same actor
			// that the interaction system considers valid
			if (HitActor && HitActor == FocusedActor)
			{
				bIsAimingAtInteractable = true;
				TargetColor = InteractableColor;
				TargetSize = InteractableSize;
				return;
			}
		}
	}

	bIsAimingAtInteractable = false;
	TargetColor = DefaultColor;
	TargetSize = DefaultSize;
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

	// Apply position (offset from center, alignment handles centering)
	CrosshairSlot->SetPosition(CurrentScreenPosition);

	// Apply size
	CrosshairImage->SetDesiredSizeOverride(CurrentSize);

	// Apply color
	CrosshairImage->SetColorAndOpacity(CurrentColor);
}


void UTFCrosshairWidget::UpdateVisibility()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	ATFPlayerController* TFPC = Cast<ATFPlayerController>(PC);

	// Default to showing crosshair unless UI is explicitly blocking
	bool bShouldHide = false;

	if (bHideWhenUIOpen && TFPC)
	{
		// Only hide for inventory and container, NOT for confirm dialogs
		bShouldHide = TFPC->IsInventoryOpen() || TFPC->IsContainerOpen();
	}

	// Control visibility through the CrosshairImage directly instead of the widget
	// This keeps NativeTick running so we can detect when to show again
	if (CrosshairImage)
	{
		if (bShouldHide)
		{
			if (CrosshairImage->GetVisibility() != ESlateVisibility::Hidden)
			{
				CrosshairImage->SetVisibility(ESlateVisibility::Hidden);
			}
		}
		else
		{
			if (CrosshairImage->GetVisibility() != ESlateVisibility::SelfHitTestInvisible)
			{
				CrosshairImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			}
		}
	}
}
