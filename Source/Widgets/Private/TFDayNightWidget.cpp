// Fill out your copyright notice in the Description page of Project Settings.

#include "TFDayNightWidget.h"
#include "TFDayNightCycle.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"

void UTFDayNightWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize day night cycle reference
	InitializeDayNightCycle();
}

void UTFDayNightWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CachedDayNightCycle)
	{
		// Try to find cycle if not cached
		InitializeDayNightCycle();
		return;
	}

	// Update displays every tick for smooth updates
	float CurrentTime = CachedDayNightCycle->GetCurrentTimeHours();
	UpdateTimeDisplay(CurrentTime);
	UpdateTimeColor(CurrentTime);
}

void UTFDayNightWidget::InitializeDayNightCycle()
{
	// Find the day night cycle actor in the world
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATFDayNightCycle::StaticClass(), FoundActors);

	if (FoundActors.Num() > 0)
	{
		CachedDayNightCycle = Cast<ATFDayNightCycle>(FoundActors[0]);

		if (CachedDayNightCycle)
		{
			// Bind to events
			CachedDayNightCycle->OnTimeChanged.AddUObject(this, &UTFDayNightWidget::OnTimeChanged);
			CachedDayNightCycle->OnDayChanged.AddUObject(this, &UTFDayNightWidget::OnDayChanged);
			CachedDayNightCycle->OnDayNightStateChanged.AddUObject(this, &UTFDayNightWidget::OnDayNightStateChanged);

			// Initialize display
			UpdateTimeDisplay(CachedDayNightCycle->GetCurrentTimeHours());
			UpdateDayDisplay(CachedDayNightCycle->GetCurrentDay());
			UpdateTimeColor(CachedDayNightCycle->GetCurrentTimeHours());
			UpdateDayNightIcon(CachedDayNightCycle->IsDay());

			bLastWasDay = CachedDayNightCycle->IsDay();
		}
	}
}

void UTFDayNightWidget::UpdateTimeDisplay(float CurrentTimeHours)
{
	if (!TimeText || !CachedDayNightCycle)
	{
		return;
	}

	// Always use 24h format (HH:MM)
	TimeText->SetText(FText::FromString(CachedDayNightCycle->GetFormattedTime()));

	// Update seconds text if available (HH:MM:SS)
	if (TimeWithSecondsText)
	{
		TimeWithSecondsText->SetText(FText::FromString(CachedDayNightCycle->GetFormattedTimeWithSeconds()));
	}
}

void UTFDayNightWidget::UpdateDayDisplay(int32 CurrentDay)
{
	if (!DayText)
	{
		return;
	}

	// Replace {day} placeholder with actual day number
	FString FormattedDay = DayTextFormat.Replace(TEXT("{day}"), *FString::FromInt(CurrentDay));
	DayText->SetText(FText::FromString(FormattedDay));
}

void UTFDayNightWidget::UpdateTimeColor(float CurrentTimeHours)
{
	if (!TimeText)
	{
		return;
	}

	FLinearColor Color = GetTimeColor(CurrentTimeHours);
	TimeText->SetColorAndOpacity(FSlateColor(Color));

	// Apply same color to day text if available
	if (DayText)
	{
		DayText->SetColorAndOpacity(FSlateColor(Color));
	}

	// Apply same color to seconds text if available
	if (TimeWithSecondsText)
	{
		TimeWithSecondsText->SetColorAndOpacity(FSlateColor(Color));
	}
}

void UTFDayNightWidget::UpdateDayNightIcon(bool bIsDay)
{
	if (!DayNightIcon)
	{
		return;
	}

	UTexture2D* IconTexture = bIsDay ? DayIconTexture : NightIconTexture;
	if (IconTexture)
	{
		DayNightIcon->SetBrushFromTexture(IconTexture);
	}
}

FLinearColor UTFDayNightWidget::GetTimeColor(float CurrentTimeHours) const
{
	// Dawn transition (night -> day)
	if (CurrentTimeHours >= DawnStartHour && CurrentTimeHours < DawnEndHour)
	{
		float Alpha = (CurrentTimeHours - DawnStartHour) / (DawnEndHour - DawnStartHour);
		return FMath::Lerp(NightTimeColor, TransitionColor, Alpha * 2.0f); // First half
	}

	// Morning (after dawn)
	if (CurrentTimeHours >= DawnEndHour && CurrentTimeHours < (DawnEndHour + 1.0f))
	{
		float Alpha = CurrentTimeHours - DawnEndHour;
		return FMath::Lerp(TransitionColor, DayTimeColor, Alpha);
	}

	// Daytime
	if (CurrentTimeHours >= (DawnEndHour + 1.0f) && CurrentTimeHours < DuskStartHour)
	{
		return DayTimeColor;
	}

	// Dusk transition (day -> night)
	if (CurrentTimeHours >= DuskStartHour && CurrentTimeHours < DuskEndHour)
	{
		float Alpha = (CurrentTimeHours - DuskStartHour) / (DuskEndHour - DuskStartHour);
		if (Alpha < 0.5f)
		{
			return FMath::Lerp(DayTimeColor, TransitionColor, Alpha * 2.0f);
		}
		else
		{
			return FMath::Lerp(TransitionColor, NightTimeColor, (Alpha - 0.5f) * 2.0f);
		}
	}

	// Nighttime
	return NightTimeColor;
}

void UTFDayNightWidget::OnTimeChanged(float CurrentTimeHours)
{
	UpdateTimeDisplay(CurrentTimeHours);
	UpdateTimeColor(CurrentTimeHours);
}

void UTFDayNightWidget::OnDayChanged(int32 CurrentDay)
{
	UpdateDayDisplay(CurrentDay);
}

void UTFDayNightWidget::OnDayNightStateChanged(bool bIsDay)
{
	UpdateDayNightIcon(bIsDay);
	bLastWasDay = bIsDay;
}

void UTFDayNightWidget::SetDayNightCycle(ATFDayNightCycle* NewDayNightCycle)
{
	// Unbind from old cycle
	if (CachedDayNightCycle)
	{
		CachedDayNightCycle->OnTimeChanged.RemoveAll(this);
		CachedDayNightCycle->OnDayChanged.RemoveAll(this);
		CachedDayNightCycle->OnDayNightStateChanged.RemoveAll(this);
	}

	// Set new cycle
	CachedDayNightCycle = NewDayNightCycle;

	// Bind to new cycle
	if (CachedDayNightCycle)
	{
		CachedDayNightCycle->OnTimeChanged.AddUObject(this, &UTFDayNightWidget::OnTimeChanged);
		CachedDayNightCycle->OnDayChanged.AddUObject(this, &UTFDayNightWidget::OnDayChanged);
		CachedDayNightCycle->OnDayNightStateChanged.AddUObject(this, &UTFDayNightWidget::OnDayNightStateChanged);

		// Initialize display
		UpdateTimeDisplay(CachedDayNightCycle->GetCurrentTimeHours());
		UpdateDayDisplay(CachedDayNightCycle->GetCurrentDay());
		UpdateTimeColor(CachedDayNightCycle->GetCurrentTimeHours());
		UpdateDayNightIcon(CachedDayNightCycle->IsDay());

		bLastWasDay = CachedDayNightCycle->IsDay();
	}
}

FString UTFDayNightWidget::GetCurrentTimeString() const
{
	if (!CachedDayNightCycle)
	{
		return TEXT("--:--");
	}

	return CachedDayNightCycle->GetFormattedTime();
}

int32 UTFDayNightWidget::GetCurrentDay() const
{
	return CachedDayNightCycle ? CachedDayNightCycle->GetCurrentDay() : 0;
}

bool UTFDayNightWidget::IsDay() const
{
	return CachedDayNightCycle ? CachedDayNightCycle->IsDay() : true;
}
