// Copyright TF Project. All Rights Reserved.

#include "TFStaminaWidget.h"
#include "TFStaminaComponent.h"
#include "TFCharacterBase.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "TFPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

void UTFStaminaWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize stamina component
	InitializeStaminaComponent();

	// Hide exhaustion warning initially
	if (ExhaustionWarning)
	{
		ExhaustionWarning->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UTFStaminaWidget::NativeDestruct()
{
	// Unbind from stamina component to prevent crashes
	if (CachedStaminaComponent)
	{
		CachedStaminaComponent->OnStaminaChanged.RemoveAll(this);
		CachedStaminaComponent->OnStaminaDepleted.RemoveAll(this);
		CachedStaminaComponent->OnStaminaRecovered.RemoveAll(this);
		CachedStaminaComponent = nullptr;
	}

	Super::NativeDestruct();
}

void UTFStaminaWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CachedStaminaComponent || !StaminaBar)
	{
		return;
	}

	float StaminaPercent = CachedStaminaComponent->GetStaminaPercent();

	// Update color
	UpdateStaminaColor(StaminaPercent);

	// Update pulse effect
	if (bEnablePulseEffect)
	{
		UpdatePulseEffect(InDeltaTime, StaminaPercent);
	}

	// Update visibility
	if (bHideWhenFull)
	{
		UpdateVisibility(StaminaPercent, InDeltaTime);
	}
}

void UTFStaminaWidget::InitializeStaminaComponent()
{
	// Get player character
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		return;
	}

	// Try to cast to TFCharacterBase
	ATFPlayerCharacter* Character = Cast<ATFPlayerCharacter>(PlayerPawn);
	if (!Character)
	{
		return;
	}

	// Get stamina component
	CachedStaminaComponent = Character->GetStaminaComponent();
	if (!CachedStaminaComponent)
	{
		return;
	}

	// Bind to events
	CachedStaminaComponent->OnStaminaChanged.AddUObject(this, &UTFStaminaWidget::OnStaminaChanged);
	CachedStaminaComponent->OnStaminaDepleted.AddUObject(this, &UTFStaminaWidget::OnExhaustion);
	CachedStaminaComponent->OnStaminaRecovered.AddUObject(this, &UTFStaminaWidget::OnRecovery);

	// Initialize display
	UpdateStaminaBar(CachedStaminaComponent->GetCurrentStamina(), CachedStaminaComponent->GetMaxStamina());
}

void UTFStaminaWidget::UpdateStaminaBar(float CurrentStamina, float MaxStamina)
{
	if (!StaminaBar)
	{
		return;
	}

	// Update progress bar
	float Percent = MaxStamina > 0.0f ? (CurrentStamina / MaxStamina) : 0.0f;
	StaminaBar->SetPercent(Percent);

	// Update text if available
	if (StaminaText)
	{
		FText StaminaDisplayText = FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurrentStamina, MaxStamina));
		StaminaText->SetText(StaminaDisplayText);
	}

	// Reset hide timer when stamina changes
	if (Percent < 1.0f)
	{
		HideTimer = 0.0f;
		if (!bIsVisible)
		{
			SetVisibility(ESlateVisibility::Visible);
			bIsVisible = true;
		}
	}
}

void UTFStaminaWidget::UpdateStaminaColor(float StaminaPercent)
{
	if (!StaminaBar)
	{
		return;
	}

	FLinearColor TargetColor;

	// Determine color based on thresholds
	if (StaminaPercent < LowStaminaThreshold)
	{
		TargetColor = LowStaminaColor;
	}
	else if (StaminaPercent < MediumStaminaThreshold)
	{
		// Interpolate between low and medium
		float Alpha = (StaminaPercent - LowStaminaThreshold) / (MediumStaminaThreshold - LowStaminaThreshold);
		TargetColor = FMath::Lerp(LowStaminaColor, MediumStaminaColor, Alpha);
	}
	else
	{
		// Interpolate between medium and high
		float Alpha = (StaminaPercent - MediumStaminaThreshold) / (1.0f - MediumStaminaThreshold);
		TargetColor = FMath::Lerp(MediumStaminaColor, HighStaminaColor, Alpha);
	}

	StaminaBar->SetFillColorAndOpacity(TargetColor);
}

void UTFStaminaWidget::UpdatePulseEffect(float DeltaTime, float StaminaPercent)
{
	if (!StaminaBar || StaminaPercent > LowStaminaThreshold)
	{
		return;
	}

	// Update pulse timer
	PulseTimer += DeltaTime * PulseSpeed;

	// Calculate pulse intensity (sine wave: 0.0 to 1.0)
	float PulseIntensity = (FMath::Sin(PulseTimer) + 1.0f) * 0.5f;

	// Apply pulse to opacity
	float PulseOpacity = FMath::Lerp(PulseBaseOpacity, 1.0f, PulseIntensity);

	FLinearColor CurrentColor = StaminaBar->GetFillColorAndOpacity();
	CurrentColor.A = PulseOpacity;
	StaminaBar->SetFillColorAndOpacity(CurrentColor);
}

void UTFStaminaWidget::UpdateVisibility(float StaminaPercent, float DeltaTime)
{
	// If stamina is full, start hide timer
	if (StaminaPercent >= 1.0f)
	{
		HideTimer += DeltaTime;

		if (HideTimer >= HideDelay && bIsVisible)
		{
			SetVisibility(ESlateVisibility::Hidden);
			bIsVisible = false;
		}
	}
	else
	{
		// Reset timer and show if hidden
		HideTimer = 0.0f;
		if (!bIsVisible)
		{
			SetVisibility(ESlateVisibility::Visible);
			bIsVisible = true;
		}
	}
}

void UTFStaminaWidget::OnStaminaChanged(float CurrentStamina, float MaxStamina)
{
	UpdateStaminaBar(CurrentStamina, MaxStamina);
}

void UTFStaminaWidget::OnExhaustion()
{
	// Show exhaustion warning
	if (ExhaustionWarning)
	{
		ExhaustionWarning->SetVisibility(ESlateVisibility::Visible);
	}

	// Optional: Play sound or animation
	// PlayAnimation(ExhaustionAnimation);
}

void UTFStaminaWidget::OnRecovery()
{
	// Hide exhaustion warning
	if (ExhaustionWarning)
	{
		ExhaustionWarning->SetVisibility(ESlateVisibility::Hidden);
	}

	// Optional: Play sound or animation
	// PlayAnimation(RecoveryAnimation);
}

void UTFStaminaWidget::SetStaminaComponent(UTFStaminaComponent* NewStaminaComponent)
{
	// Unbind from old component
	if (CachedStaminaComponent)
	{
		CachedStaminaComponent->OnStaminaChanged.RemoveAll(this);
		CachedStaminaComponent->OnStaminaDepleted.RemoveAll(this);
		CachedStaminaComponent->OnStaminaRecovered.RemoveAll(this);
	}

	// Set new component
	CachedStaminaComponent = NewStaminaComponent;

	// Bind to new component
	if (CachedStaminaComponent)
	{
		CachedStaminaComponent->OnStaminaChanged.AddUObject(this, &UTFStaminaWidget::OnStaminaChanged);
		CachedStaminaComponent->OnStaminaDepleted.AddUObject(this, &UTFStaminaWidget::OnExhaustion);
		CachedStaminaComponent->OnStaminaRecovered.AddUObject(this, &UTFStaminaWidget::OnRecovery);

		// Initialize display
		UpdateStaminaBar(CachedStaminaComponent->GetCurrentStamina(), CachedStaminaComponent->GetMaxStamina());
	}
}

float UTFStaminaWidget::GetStaminaPercent() const
{
	return CachedStaminaComponent ? CachedStaminaComponent->GetStaminaPercent() : 0.0f;
}

bool UTFStaminaWidget::IsExhausted() const
{
	return CachedStaminaComponent ? CachedStaminaComponent->IsExhausted() : false;
}
