// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TFCrosshairWidget.generated.h"

class UImage;
class UCanvasPanel;
class UCanvasPanelSlot;
class ATFPlayerCharacter;

/**
 * Crosshair HUD Widget
 * Displays a crosshair that follows the line trace impact point
 */
UCLASS()
class WIDGETS_API UTFCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

#pragma region Widget Bindings

	/** Main crosshair image */
	UPROPERTY(meta = (BindWidget))
	UImage* CrosshairImage;

	/** Canvas panel containing the crosshair (for positioning) */
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* CrosshairCanvas;

#pragma endregion Widget Bindings

#pragma region Trace Settings

	/** Maximum distance for line trace */
	UPROPERTY(EditAnywhere, Category = "Crosshair|Trace", meta = (ClampMin = "100.0", ClampMax = "50000.0"))
	float TraceDistance = 10000.0f;

	/** Collision channel for trace */
	UPROPERTY(EditAnywhere, Category = "Crosshair|Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	/** Use complex collision for trace */
	UPROPERTY(EditAnywhere, Category = "Crosshair|Trace")
	bool bTraceComplex = false;

#pragma endregion Trace Settings

#pragma region Visual Settings

	/** Default crosshair color */
	UPROPERTY(EditAnywhere, Category = "Crosshair|Colors")
	FLinearColor DefaultColor = FLinearColor::White;

	/** Color when aiming at an interactable */
	UPROPERTY(EditAnywhere, Category = "Crosshair|Colors")
	FLinearColor InteractableColor = FLinearColor::Green;

	/** Default crosshair size */
	UPROPERTY(EditAnywhere, Category = "Crosshair|Size")
	FVector2D DefaultSize = FVector2D(16.0f, 16.0f);

	/** Size when aiming at an interactable */
	UPROPERTY(EditAnywhere, Category = "Crosshair|Size")
	FVector2D InteractableSize = FVector2D(24.0f, 24.0f);

	/** Size interpolation speed */
	UPROPERTY(EditAnywhere, Category = "Crosshair|Animation", meta = (ClampMin = "1.0", ClampMax = "30.0"))
	float SizeInterpSpeed = 10.0f;

	/** Color interpolation speed */
	UPROPERTY(EditAnywhere, Category = "Crosshair|Animation", meta = (ClampMin = "1.0", ClampMax = "30.0"))
	float ColorInterpSpeed = 10.0f;

	/** Position smoothing (0 = no smoothing, higher = smoother) */
	UPROPERTY(EditAnywhere, Category = "Crosshair|Animation", meta = (ClampMin = "0.0", ClampMax = "30.0"))
	float PositionSmoothSpeed = 15.0f;

	/** Hide crosshair when UI is blocking input */
	UPROPERTY(EditAnywhere, Category = "Crosshair|Visibility")
	bool bHideWhenUIOpen = true;

#pragma endregion Visual Settings

private:

	/** Cached player character reference */
	UPROPERTY()
	TWeakObjectPtr<ATFPlayerCharacter> CachedPlayerCharacter;

	/** Cached canvas slot for the crosshair image */
	UPROPERTY()
	UCanvasPanelSlot* CrosshairSlot;

	/** Current crosshair screen position */
	FVector2D CurrentScreenPosition;

	/** Target crosshair screen position */
	FVector2D TargetScreenPosition;

	/** Current crosshair size */
	FVector2D CurrentSize;

	/** Target crosshair size */
	FVector2D TargetSize;

	/** Current crosshair color */
	FLinearColor CurrentColor;

	/** Target crosshair color */
	FLinearColor TargetColor;

	/** Is currently aiming at an interactable */
	bool bIsAimingAtInteractable = false;

	/** Current world hit location */
	FVector CurrentHitLocation;

	/** Did the trace hit anything */
	bool bHasHit = false;

protected:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Find and cache player character */
	void InitializePlayerCharacter();

	/** Perform line trace from camera */
	bool PerformTrace(FHitResult& OutHitResult);

	/** Get trace start and end points from player camera */
	bool GetTracePoints(FVector& TraceStart, FVector& TraceEnd) const;

	/** Update crosshair position based on hit result */
	void UpdateCrosshairPosition(const FHitResult& HitResult, float DeltaTime);

	/** Update crosshair position when no hit */
	void UpdateCrosshairPositionNoHit(float DeltaTime);

	/** Update crosshair visual state based on what's being aimed at */
	void UpdateCrosshairVisuals(const FHitResult& HitResult, float DeltaTime);

	/** Interpolate crosshair properties */
	void InterpolateCrosshairProperties(float DeltaTime);

	/** Apply current properties to widget */
	void ApplyCrosshairProperties();

	/** Update visibility based on UI state */
	void UpdateVisibility();

public:

	/** Get the current world hit location */
	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	FVector GetCurrentHitLocation() const { return CurrentHitLocation; }

	/** Check if crosshair is aiming at something */
	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	bool HasHit() const { return bHasHit; }

	/** Check if aiming at an interactable */
	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	bool IsAimingAtInteractable() const { return bIsAimingAtInteractable; }

	/** Get the current screen position of the crosshair */
	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	FVector2D GetScreenPosition() const { return CurrentScreenPosition; }
};
