// Copyright TF Project. All Rights Reserved.

#include "TFLockProgressWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UTFLockProgressWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Hide widget initially
	SetVisibility(ESlateVisibility::Hidden);
	CurrentFadeAlpha = 0.0f;
	TargetFadeAlpha = 0.0f;

	// Initialize progress bar
	if (LockProgressBar)
	{
		LockProgressBar->SetPercent(0.0f);
	}
}

void UTFLockProgressWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update fade animation
	UpdateFadeAnimation(InDeltaTime);
}

void UTFLockProgressWidget::StartProgress(float Duration, bool bUnlocking)
{
	if (Duration <= 0.0f)
	{
		return;
	}

	bIsActive = true;
	bIsUnlocking = bUnlocking;
	TotalDuration = Duration;
	ElapsedTime = 0.0f;

	// Apply visual style
	ApplyVisualStyle();

	// Update progress bar
	UpdateProgressBar();

	// Start fade in
	TargetFadeAlpha = 1.0f;
	SetVisibility(ESlateVisibility::Visible);
}

void UTFLockProgressWidget::UpdateProgress(float NewElapsedTime)
{
	if (!bIsActive)
	{
		return;
	}

	ElapsedTime = FMath::Clamp(NewElapsedTime, 0.0f, TotalDuration);
	UpdateProgressBar();
}

void UTFLockProgressWidget::CompleteProgress()
{
	if (!bIsActive)
	{
		return;
	}

	// Set to 100%
	ElapsedTime = TotalDuration;
	UpdateProgressBar();

	// Start fade out
	TargetFadeAlpha = 0.0f;
	bIsActive = false;

	OnLockProgressComplete.Broadcast();
}

void UTFLockProgressWidget::CancelProgress()
{
	if (!bIsActive)
	{
		return;
	}

	// Start fade out
	TargetFadeAlpha = 0.0f;
	bIsActive = false;

	OnLockProgressCancelled.Broadcast();
}

void UTFLockProgressWidget::KeyBrokenProgress()
{
	if (!bIsActive)
	{
		return;
	}

	// Change color to broken key color
	if (LockProgressBar)
	{
		LockProgressBar->SetFillColorAndOpacity(KeyBrokenColor);
	}

	// Change text to key broken
	if (ActionText)
	{
		ActionText->SetText(KeyBrokenText);
	}

	// Start fade out
	TargetFadeAlpha = 0.0f;
	bIsActive = false;

	OnLockProgressKeyBroken.Broadcast();
}

float UTFLockProgressWidget::GetProgressPercent() const
{
	if (TotalDuration <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(ElapsedTime / TotalDuration, 0.0f, 1.0f);
}

void UTFLockProgressWidget::UpdateProgressBar()
{
	if (!LockProgressBar)
	{
		return;
	}

	float Percent = GetProgressPercent();
	LockProgressBar->SetPercent(Percent);
}

void UTFLockProgressWidget::UpdateFadeAnimation(float DeltaTime)
{
	if (FMath::IsNearlyEqual(CurrentFadeAlpha, TargetFadeAlpha, 0.01f))
	{
		CurrentFadeAlpha = TargetFadeAlpha;

		// Hide when fully faded out
		if (CurrentFadeAlpha <= 0.0f)
		{
			SetVisibility(ESlateVisibility::Hidden);

			// Reset state
			ElapsedTime = 0.0f;
			TotalDuration = 0.0f;

			if (LockProgressBar)
			{
				LockProgressBar->SetPercent(0.0f);
			}
		}

		return;
	}

	// Calculate fade speed
	float FadeSpeed = FadeDuration > 0.0f ? (1.0f / FadeDuration) : 100.0f;

	// Interpolate alpha
	if (TargetFadeAlpha > CurrentFadeAlpha)
	{
		CurrentFadeAlpha = FMath::Min(CurrentFadeAlpha + (FadeSpeed * DeltaTime), TargetFadeAlpha);
	}
	else
	{
		CurrentFadeAlpha = FMath::Max(CurrentFadeAlpha - (FadeSpeed * DeltaTime), TargetFadeAlpha);
	}

	// Apply alpha to widget
	SetRenderOpacity(CurrentFadeAlpha);
}

void UTFLockProgressWidget::ApplyVisualStyle()
{
	// Set progress bar color based on action type
	if (LockProgressBar)
	{
		FLinearColor FillColor = bIsUnlocking ? UnlockColor : LockColor;
		LockProgressBar->SetFillColorAndOpacity(FillColor);
	}

	// Set action text
	if (ActionText)
	{
		FText DisplayText = bIsUnlocking ? UnlockingText : LockingText;
		ActionText->SetText(DisplayText);
	}
}
