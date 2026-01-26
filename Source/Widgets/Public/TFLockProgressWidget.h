// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TFLockProgressWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;

DECLARE_MULTICAST_DELEGATE(FOnLockProgressComplete);
DECLARE_MULTICAST_DELEGATE(FOnLockProgressCancelled);
DECLARE_MULTICAST_DELEGATE(FOnLockProgressKeyBroken);

/**
 * Lock/Unlock Progress Widget
 * Displays progress bar during door lock/unlock actions
 */
UCLASS()
class WIDGETS_API UTFLockProgressWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

#pragma region Widget Bindings

	/** Main progress bar */
	UPROPERTY(meta = (BindWidget))
	UProgressBar* LockProgressBar;

	/** Optional action text (e.g., "Unlocking..." or "Locking...") */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ActionText;

	/** Optional key icon */
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* KeyIcon;

#pragma endregion Widget Bindings

#pragma region Visual Settings

	/** Color when unlocking */
	UPROPERTY(EditAnywhere, Category = "Lock|Colors")
	FLinearColor UnlockColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);

	/** Color when locking */
	UPROPERTY(EditAnywhere, Category = "Lock|Colors")
	FLinearColor LockColor = FLinearColor(0.8f, 0.2f, 0.2f, 1.0f);

	/** Background color for progress bar */
	UPROPERTY(EditAnywhere, Category = "Lock|Colors")
	FLinearColor BackgroundColor = FLinearColor(0.1f, 0.1f, 0.1f, 0.8f);

	/** Text to display when unlocking */
	UPROPERTY(EditAnywhere, Category = "Lock|Text")
	FText UnlockingText = FText::FromString(TEXT("Unlocking..."));

	/** Text to display when locking */
	UPROPERTY(EditAnywhere, Category = "Lock|Text")
	FText LockingText = FText::FromString(TEXT("Locking..."));

	/** Text to display when key breaks */
	UPROPERTY(EditAnywhere, Category = "Lock|Text")
	FText KeyBrokenText = FText::FromString(TEXT("Key Broken!"));

	/** Color when key breaks */
	UPROPERTY(EditAnywhere, Category = "Lock|Colors")
	FLinearColor KeyBrokenColor = FLinearColor(1.0f, 0.3f, 0.0f, 1.0f);

	/** Fade in/out duration */
	UPROPERTY(EditAnywhere, Category = "Lock|Animation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FadeDuration = 0.15f;

#pragma endregion Visual Settings

private:

	/** Is the widget currently active */
	bool bIsActive = false;

	/** Is this an unlock action (true) or lock action (false) */
	bool bIsUnlocking = true;

	/** Total duration of the action */
	float TotalDuration = 0.0f;

	/** Current elapsed time */
	float ElapsedTime = 0.0f;

	/** Current fade alpha for animation */
	float CurrentFadeAlpha = 0.0f;

	/** Target fade alpha */
	float TargetFadeAlpha = 0.0f;

protected:

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Update progress bar visual */
	void UpdateProgressBar();

	/** Update fade animation */
	void UpdateFadeAnimation(float DeltaTime);

	/** Apply visual style based on action type */
	void ApplyVisualStyle();

public:

	FOnLockProgressComplete OnLockProgressComplete;
	FOnLockProgressCancelled OnLockProgressCancelled;
	FOnLockProgressKeyBroken OnLockProgressKeyBroken;

	/**
	 * Start showing lock/unlock progress
	 * @param Duration Total duration of the action in seconds
	 * @param bUnlocking True if unlocking, false if locking
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock Progress")
	void StartProgress(float Duration, bool bUnlocking);

	/**
	 * Update progress with current elapsed time
	 * @param NewElapsedTime Current elapsed time in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock Progress")
	void UpdateProgress(float NewElapsedTime);

	/**
	 * Complete the progress (called when action finishes successfully)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock Progress")
	void CompleteProgress();

	/**
	 * Cancel the progress (called when player releases key early)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock Progress")
	void CancelProgress();

	/**
	 * Key broken during unlock attempt (shows broken key feedback)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock Progress")
	void KeyBrokenProgress();

	/**
	 * Get current progress percentage (0.0 - 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock Progress")
	float GetProgressPercent() const;

	/**
	 * Check if the widget is currently active
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock Progress")
	bool IsProgressActive() const { return bIsActive; }
};
