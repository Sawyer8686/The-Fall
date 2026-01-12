// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TFStaminaWidget.generated.h"

class UTFStaminaComponent;
class UProgressBar;
class UTextBlock;
class UImage;

/**
 * Stamina HUD Widget
 * Displays stamina bar with visual feedback
 */
UCLASS()
class WIDGETS_API UTFStaminaWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

#pragma region Widget Bindings

	/** Main stamina progress bar */
	UPROPERTY(meta = (BindWidget))
	UProgressBar* StaminaBar;

	/** Optional stamina text display */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* StaminaText;

	/** Optional exhaustion warning image */
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* ExhaustionWarning;

#pragma endregion Widget Bindings

#pragma region Visual Settings

	/** Color when stamina is high */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Colors")
	FLinearColor HighStaminaColor = FLinearColor::Green;

	/** Color when stamina is medium */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Colors")
	FLinearColor MediumStaminaColor = FLinearColor::Yellow;

	/** Color when stamina is low */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Colors")
	FLinearColor LowStaminaColor = FLinearColor::Red;

	/** Threshold for medium stamina (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Colors", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MediumStaminaThreshold = 0.5f;

	/** Threshold for low stamina (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Colors", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowStaminaThreshold = 0.25f;

	/** Enable pulsing effect when low stamina */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Effects")
	bool bEnablePulseEffect = true;

	/** Pulse speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Effects", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float PulseSpeed = 2.0f;

	/** Hide stamina bar when full */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Visibility")
	bool bHideWhenFull = true;

	/** Delay before hiding when full (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Visibility", meta = (ClampMin = "0.0"))
	float HideDelay = 2.0f;

#pragma endregion Visual Settings

private:

	/** Cached stamina component reference */
	UPROPERTY()
	UTFStaminaComponent* CachedStaminaComponent;

	/** Timer for pulse animation */
	float PulseTimer = 0.0f;

	/** Timer for hiding bar */
	float HideTimer = 0.0f;

	/** Is bar currently visible */
	bool bIsVisible = true;

protected:

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Find and cache stamina component from owning player */
	void InitializeStaminaComponent();

	/** Update stamina bar visual */
	void UpdateStaminaBar(float CurrentStamina, float MaxStamina);

	/** Update stamina color based on percentage */
	void UpdateStaminaColor(float StaminaPercent);

	/** Handle pulse effect for low stamina */
	void UpdatePulseEffect(float DeltaTime, float StaminaPercent);

	/** Update visibility based on stamina state */
	void UpdateVisibility(float StaminaPercent, float DeltaTime);

	/** Callback for stamina changes */
	UFUNCTION()
	void OnStaminaChanged(float CurrentStamina, float MaxStamina);

	/** Callback for exhaustion */
	UFUNCTION()
	void OnExhaustion();

	/** Callback for recovery */
	UFUNCTION()
	void OnRecovery();

public:

	/** Manually set stamina component (optional, auto-detects by default) */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetStaminaComponent(UTFStaminaComponent* NewStaminaComponent);

	/** Get current stamina percentage (for animations) */
	UFUNCTION(BlueprintPure, Category = "Stamina")
	float GetStaminaPercent() const;

	/** Check if currently exhausted */
	UFUNCTION(BlueprintPure, Category = "Stamina")
	bool IsExhausted() const;
};