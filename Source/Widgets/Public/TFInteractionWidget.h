// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/TFInteractableInterface.h"
#include "TFInteractionWidget.generated.h"

class UTFInteractionComponent;
class UTextBlock;
class UImage;
class UProgressBar;

/**
 * Interaction Widget
 * Displays interaction prompts when player looks at interactable objects
 */
UCLASS()
class WIDGETS_API UTFInteractionWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

#pragma region Widget Bindings

	/** Main interaction text (e.g., "Press E to Open Door") */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* InteractionText;

	/** Secondary text (e.g., "Locked" or "Requires Key") */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* SecondaryText;

	/** Interaction icon */
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* InteractionIcon;

	/** Hold progress bar (for hold interactions) */
	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* HoldProgressBar;

	/** Input key hint text */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* KeyHintText;

#pragma endregion Widget Bindings

#pragma region Settings

	/** Input key to display (e.g., "E", "F", etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText InputKeyHint = FText::FromString("E");

	/** Show hold progress bar */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bShowHoldProgress = true;

	/** Fade in/out animation duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Animation")
	float FadeDuration = 0.2f;

#pragma endregion Settings

private:

	/** Cached interaction component reference */
	UPROPERTY()
	UTFInteractionComponent* CachedInteractionComponent;

	/** Is widget currently visible */
	bool bIsShowing = false;

	/** Current fade alpha */
	float CurrentAlpha = 0.0f;

	/** Target fade alpha */
	float TargetAlpha = 0.0f;

protected:

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Initialize interaction component reference */
	void InitializeInteractionComponent();

	/** Update widget display with interaction data */
	void UpdateInteractionDisplay(const FInteractionData& Data);

	/** Update hold progress bar */
	void UpdateHoldProgress(float Progress);

	/** Show widget with fade in */
	void ShowWidget();

	/** Hide widget with fade out */
	void HideWidget();

	/** Update fade animation */
	void UpdateFade(float DeltaTime);

	/** Callbacks for interaction events */
	UFUNCTION()
	void OnInteractionChanged(AActor* InteractableActor, FInteractionData InteractionData);

	UFUNCTION()
	void OnInteractionLost();

public:

	/** Manually set interaction component (optional, auto-detects by default) */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractionComponent(UTFInteractionComponent* NewInteractionComponent);

	/** Check if widget is currently showing */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	FORCEINLINE bool IsShowing() const { return bIsShowing; }
};