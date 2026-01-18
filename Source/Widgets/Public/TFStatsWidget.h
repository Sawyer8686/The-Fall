// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TFStatsWidget.generated.h"

class UTFStatsComponent;
class UProgressBar;
class UTextBlock;
class UImage;

/**
 * Stats HUD Widget
 * Displays hunger and thirst bars with visual feedback
 */
UCLASS()
class WIDGETS_API UTFStatsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

#pragma region Widget Bindings

	/** Hunger progress bar */
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HungerBar;

	/** Thirst progress bar */
	UPROPERTY(meta = (BindWidget))
	UProgressBar* ThirstBar;

	/** Optional hunger text display */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* HungerText;

	/** Optional thirst text display */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ThirstText;

	/** Optional hunger warning icon */
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* HungerWarning;

	/** Optional thirst warning icon */
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* ThirstWarning;

#pragma endregion Widget Bindings

#pragma region Hunger Visual Settings

	/** Color when hunger is high */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Hunger|Colors")
	FLinearColor HighHungerColor = FLinearColor(0.2f, 0.8f, 0.2f);

	/** Color when hunger is medium */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Hunger|Colors")
	FLinearColor MediumHungerColor = FLinearColor(0.8f, 0.6f, 0.0f);

	/** Color when hunger is low */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Hunger|Colors")
	FLinearColor LowHungerColor = FLinearColor(0.8f, 0.2f, 0.0f);

	/** Threshold for medium hunger (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Hunger|Colors", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MediumHungerThreshold = 0.5f;

	/** Threshold for low hunger (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Hunger|Colors", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowHungerThreshold = 0.25f;

#pragma endregion Hunger Visual Settings

#pragma region Thirst Visual Settings

	/** Color when thirst is high */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Thirst|Colors")
	FLinearColor HighThirstColor = FLinearColor(0.2f, 0.6f, 1.0f);

	/** Color when thirst is medium */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Thirst|Colors")
	FLinearColor MediumThirstColor = FLinearColor(0.4f, 0.4f, 0.8f);

	/** Color when thirst is low */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Thirst|Colors")
	FLinearColor LowThirstColor = FLinearColor(0.6f, 0.0f, 0.0f);

	/** Threshold for medium thirst (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Thirst|Colors", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MediumThirstThreshold = 0.5f;

	/** Threshold for low thirst (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Thirst|Colors", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowThirstThreshold = 0.25f;

#pragma endregion Thirst Visual Settings

#pragma region Effects Settings

	/** Enable pulsing effect when stats are low */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Effects")
	bool bEnablePulseEffect = true;

	/** Pulse speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Effects", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float PulseSpeed = 2.0f;

#pragma endregion Effects Settings

private:

	/** Cached stats component reference */
	UPROPERTY()
	UTFStatsComponent* CachedStatsComponent;

	/** Timer for hunger pulse animation */
	float HungerPulseTimer = 0.0f;

	/** Timer for thirst pulse animation */
	float ThirstPulseTimer = 0.0f;

protected:

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Find and cache stats component from owning player */
	void InitializeStatsComponent();

	/** Update hunger bar visual */
	void UpdateHungerBar(float CurrentHunger, float MaxHunger);

	/** Update thirst bar visual */
	void UpdateThirstBar(float CurrentThirst, float MaxThirst);

	/** Update hunger color based on percentage */
	void UpdateHungerColor(float HungerPercent);

	/** Update thirst color based on percentage */
	void UpdateThirstColor(float ThirstPercent);

	/** Handle pulse effect for low hunger */
	void UpdateHungerPulseEffect(float DeltaTime, float HungerPercent);

	/** Handle pulse effect for low thirst */
	void UpdateThirstPulseEffect(float DeltaTime, float ThirstPercent);

	/** Callback for hunger changes */
	UFUNCTION()
	void OnHungerChanged(float CurrentHunger, float MaxHunger);

	/** Callback for thirst changes */
	UFUNCTION()
	void OnThirstChanged(float CurrentThirst, float MaxThirst);

	/** Callback for stat depletion */
	UFUNCTION()
	void OnStatDepleted(FName StatName);

	/** Callback for stat critical */
	UFUNCTION()
	void OnStatCritical(FName StatName, float Percent);

public:

	/** Manually set stats component (optional, auto-detects by default) */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetStatsComponent(UTFStatsComponent* NewStatsComponent);

	/** Get current hunger percentage */
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetHungerPercent() const;

	/** Get current thirst percentage */
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetThirstPercent() const;

	/** Check if hunger is critical */
	UFUNCTION(BlueprintPure, Category = "Stats")
	bool IsHungerCritical() const;

	/** Check if thirst is critical */
	UFUNCTION(BlueprintPure, Category = "Stats")
	bool IsThirstCritical() const;
};
