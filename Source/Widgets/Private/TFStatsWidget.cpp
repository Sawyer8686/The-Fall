// Copyright TF Project. All Rights Reserved.

#include "TFStatsWidget.h"
#include "TF.h"
#include "TFStatsComponent.h"
#include "TFPlayerCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"

void UTFStatsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize stats component
	InitializeStatsComponent();

	// Hide warning icons initially
	if (HungerWarning)
	{
		HungerWarning->SetVisibility(ESlateVisibility::Hidden);
	}
	if (ThirstWarning)
	{
		ThirstWarning->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UTFStatsWidget::NativeDestruct()
{
	// Unbind from stats component to prevent crashes
	if (CachedStatsComponent)
	{
		CachedStatsComponent->OnHungerChanged.RemoveAll(this);
		CachedStatsComponent->OnThirstChanged.RemoveAll(this);
		CachedStatsComponent->OnStatDepleted.RemoveAll(this);
		CachedStatsComponent->OnStatCritical.RemoveAll(this);
		CachedStatsComponent = nullptr;
	}

	Super::NativeDestruct();
}

void UTFStatsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CachedStatsComponent)
	{
		return;
	}

	// Update hunger visuals
	if (HungerBar)
	{
		float HungerPercent = CachedStatsComponent->GetHungerPercent();
		UpdateHungerColor(HungerPercent);

		if (bEnablePulseEffect)
		{
			UpdateHungerPulseEffect(InDeltaTime, HungerPercent);
		}
	}

	// Update thirst visuals
	if (ThirstBar)
	{
		float ThirstPercent = CachedStatsComponent->GetThirstPercent();
		UpdateThirstColor(ThirstPercent);

		if (bEnablePulseEffect)
		{
			UpdateThirstPulseEffect(InDeltaTime, ThirstPercent);
		}
	}
}

void UTFStatsWidget::InitializeStatsComponent()
{
	// Get player character
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		return;
	}

	// Try to cast to TFPlayerCharacter
	ATFPlayerCharacter* Character = Cast<ATFPlayerCharacter>(PlayerPawn);
	if (!Character)
	{
		return;
	}

	// Get stats component
	CachedStatsComponent = Character->GetStatsComponent();
	if (!CachedStatsComponent)
	{
		return;
	}

	// Bind to events
	CachedStatsComponent->OnHungerChanged.AddUObject(this, &UTFStatsWidget::OnHungerChanged);
	CachedStatsComponent->OnThirstChanged.AddUObject(this, &UTFStatsWidget::OnThirstChanged);
	CachedStatsComponent->OnStatDepleted.AddUObject(this, &UTFStatsWidget::OnStatDepleted);
	CachedStatsComponent->OnStatCritical.AddUObject(this, &UTFStatsWidget::OnStatCritical);

	// Initialize display
	UpdateHungerBar(CachedStatsComponent->GetCurrentHunger(), CachedStatsComponent->GetMaxHunger());
	UpdateThirstBar(CachedStatsComponent->GetCurrentThirst(), CachedStatsComponent->GetMaxThirst());
}

void UTFStatsWidget::UpdateHungerBar(float CurrentHunger, float MaxHunger)
{
	if (!HungerBar)
	{
		return;
	}

	// Update progress bar
	float Percent = MaxHunger > 0.0f ? (CurrentHunger / MaxHunger) : 0.0f;
	HungerBar->SetPercent(Percent);

	// Update text if available
	if (HungerText)
	{
		FText HungerDisplayText = FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurrentHunger, MaxHunger));
		HungerText->SetText(HungerDisplayText);
	}
}

void UTFStatsWidget::UpdateThirstBar(float CurrentThirst, float MaxThirst)
{
	if (!ThirstBar)
	{
		return;
	}

	// Update progress bar
	float Percent = MaxThirst > 0.0f ? (CurrentThirst / MaxThirst) : 0.0f;
	ThirstBar->SetPercent(Percent);

	// Update text if available
	if (ThirstText)
	{
		FText ThirstDisplayText = FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurrentThirst, MaxThirst));
		ThirstText->SetText(ThirstDisplayText);
	}
}

void UTFStatsWidget::UpdateHungerColor(float HungerPercent)
{
	if (!HungerBar)
	{
		return;
	}

	FLinearColor TargetColor;

	// Determine color based on thresholds
	if (HungerPercent < LowHungerThreshold)
	{
		TargetColor = LowHungerColor;
	}
	else if (HungerPercent < MediumHungerThreshold)
	{
		// Interpolate between low and medium
		float Alpha = (HungerPercent - LowHungerThreshold) / (MediumHungerThreshold - LowHungerThreshold);
		TargetColor = FMath::Lerp(LowHungerColor, MediumHungerColor, Alpha);
	}
	else
	{
		// Interpolate between medium and high
		float Alpha = (HungerPercent - MediumHungerThreshold) / (1.0f - MediumHungerThreshold);
		TargetColor = FMath::Lerp(MediumHungerColor, HighHungerColor, Alpha);
	}

	HungerBar->SetFillColorAndOpacity(TargetColor);
}

void UTFStatsWidget::UpdateThirstColor(float ThirstPercent)
{
	if (!ThirstBar)
	{
		return;
	}

	FLinearColor TargetColor;

	// Determine color based on thresholds
	if (ThirstPercent < LowThirstThreshold)
	{
		TargetColor = LowThirstColor;
	}
	else if (ThirstPercent < MediumThirstThreshold)
	{
		// Interpolate between low and medium
		float Alpha = (ThirstPercent - LowThirstThreshold) / (MediumThirstThreshold - LowThirstThreshold);
		TargetColor = FMath::Lerp(LowThirstColor, MediumThirstColor, Alpha);
	}
	else
	{
		// Interpolate between medium and high
		float Alpha = (ThirstPercent - MediumThirstThreshold) / (1.0f - MediumThirstThreshold);
		TargetColor = FMath::Lerp(MediumThirstColor, HighThirstColor, Alpha);
	}

	ThirstBar->SetFillColorAndOpacity(TargetColor);
}

void UTFStatsWidget::UpdateHungerPulseEffect(float DeltaTime, float HungerPercent)
{
	if (!HungerBar || HungerPercent > LowHungerThreshold)
	{
		return;
	}

	// Update pulse timer
	HungerPulseTimer += DeltaTime * PulseSpeed;

	// Calculate pulse intensity (sine wave)
	float PulseIntensity = (FMath::Sin(HungerPulseTimer) + 1.0f) * 0.5f;

	// Apply pulse to opacity
	float PulseOpacity = FMath::Lerp(PulseBaseOpacity, 1.0f, PulseIntensity);

	FLinearColor CurrentColor = HungerBar->GetFillColorAndOpacity();
	CurrentColor.A = PulseOpacity;
	HungerBar->SetFillColorAndOpacity(CurrentColor);
}

void UTFStatsWidget::UpdateThirstPulseEffect(float DeltaTime, float ThirstPercent)
{
	if (!ThirstBar || ThirstPercent > LowThirstThreshold)
	{
		return;
	}

	// Update pulse timer
	ThirstPulseTimer += DeltaTime * PulseSpeed;

	// Calculate pulse intensity (sine wave)
	float PulseIntensity = (FMath::Sin(ThirstPulseTimer) + 1.0f) * 0.5f;

	// Apply pulse to opacity
	float PulseOpacity = FMath::Lerp(PulseBaseOpacity, 1.0f, PulseIntensity);

	FLinearColor CurrentColor = ThirstBar->GetFillColorAndOpacity();
	CurrentColor.A = PulseOpacity;
	ThirstBar->SetFillColorAndOpacity(CurrentColor);
}

void UTFStatsWidget::OnHungerChanged(float CurrentHunger, float MaxHunger)
{
	UpdateHungerBar(CurrentHunger, MaxHunger);
}

void UTFStatsWidget::OnThirstChanged(float CurrentThirst, float MaxThirst)
{
	UpdateThirstBar(CurrentThirst, MaxThirst);
}

void UTFStatsWidget::OnStatDepleted(FName StatName)
{
	if (StatName == TFStatNames::Hunger)
	{
		if (HungerWarning)
		{
			HungerWarning->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else if (StatName == TFStatNames::Thirst)
	{
		if (ThirstWarning)
		{
			ThirstWarning->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UTFStatsWidget::OnStatCritical(FName StatName, float Percent)
{
	if (StatName == TFStatNames::Hunger)
	{
		if (HungerWarning)
		{
			HungerWarning->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else if (StatName == TFStatNames::Thirst)
	{
		if (ThirstWarning)
		{
			ThirstWarning->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UTFStatsWidget::SetStatsComponent(UTFStatsComponent* NewStatsComponent)
{
	// Unbind from old component
	if (CachedStatsComponent)
	{
		CachedStatsComponent->OnHungerChanged.RemoveAll(this);
		CachedStatsComponent->OnThirstChanged.RemoveAll(this);
		CachedStatsComponent->OnStatDepleted.RemoveAll(this);
		CachedStatsComponent->OnStatCritical.RemoveAll(this);
	}

	// Set new component
	CachedStatsComponent = NewStatsComponent;

	// Bind to new component
	if (CachedStatsComponent)
	{
		CachedStatsComponent->OnHungerChanged.AddUObject(this, &UTFStatsWidget::OnHungerChanged);
		CachedStatsComponent->OnThirstChanged.AddUObject(this, &UTFStatsWidget::OnThirstChanged);
		CachedStatsComponent->OnStatDepleted.AddUObject(this, &UTFStatsWidget::OnStatDepleted);
		CachedStatsComponent->OnStatCritical.AddUObject(this, &UTFStatsWidget::OnStatCritical);

		// Initialize display
		UpdateHungerBar(CachedStatsComponent->GetCurrentHunger(), CachedStatsComponent->GetMaxHunger());
		UpdateThirstBar(CachedStatsComponent->GetCurrentThirst(), CachedStatsComponent->GetMaxThirst());
	}
}

float UTFStatsWidget::GetHungerPercent() const
{
	return CachedStatsComponent ? CachedStatsComponent->GetHungerPercent() : 0.0f;
}

float UTFStatsWidget::GetThirstPercent() const
{
	return CachedStatsComponent ? CachedStatsComponent->GetThirstPercent() : 0.0f;
}

bool UTFStatsWidget::IsHungerCritical() const
{
	return CachedStatsComponent ? CachedStatsComponent->IsHungerCritical() : false;
}

bool UTFStatsWidget::IsThirstCritical() const
{
	return CachedStatsComponent ? CachedStatsComponent->IsThirstCritical() : false;
}
