// Fill out your copyright notice in the Description page of Project Settings.

#include "TFDayNightWidget.h"
#include "TFDayNightCycle.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"

void UTFDayNightWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeDayNightCycle();
}

void UTFDayNightWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CachedDayNightCycle)
	{
		InitializeDayNightCycle();
		return;
	}

	float CurrentTime = CachedDayNightCycle->GetCurrentTimeHours();
	UpdateTimeDisplay(CurrentTime);
}

void UTFDayNightWidget::InitializeDayNightCycle()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATFDayNightCycle::StaticClass(), FoundActors);

	if (FoundActors.Num() > 0)
	{
		CachedDayNightCycle = Cast<ATFDayNightCycle>(FoundActors[0]);

		if (CachedDayNightCycle)
		{
			CachedDayNightCycle->OnTimeChanged.AddUObject(this, &UTFDayNightWidget::OnTimeChanged);
			CachedDayNightCycle->OnDayChanged.AddUObject(this, &UTFDayNightWidget::OnDayChanged);
			CachedDayNightCycle->OnDayNightStateChanged.AddUObject(this, &UTFDayNightWidget::OnDayNightStateChanged);

			UpdateTimeDisplay(CachedDayNightCycle->GetCurrentTimeHours());
			UpdateDayDisplay(CachedDayNightCycle->GetCurrentDay());
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

	TimeText->SetText(FText::FromString(CachedDayNightCycle->GetFormattedTime()));

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

	FString FormattedDay = DayTextFormat.Replace(TEXT("{day}"), *FString::FromInt(CurrentDay));
	DayText->SetText(FText::FromString(FormattedDay));
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
	if (CurrentTimeHours >= DawnStartHour && CurrentTimeHours < DawnEndHour)
	{
		float Alpha = (CurrentTimeHours - DawnStartHour) / (DawnEndHour - DawnStartHour);
		return FMath::Lerp(NightTimeColor, TransitionColor, Alpha * 2.0f); 
	}

	if (CurrentTimeHours >= DawnEndHour && CurrentTimeHours < (DawnEndHour + 1.0f))
	{
		float Alpha = CurrentTimeHours - DawnEndHour;
		return FMath::Lerp(TransitionColor, DayTimeColor, Alpha);
	}

	if (CurrentTimeHours >= (DawnEndHour + 1.0f) && CurrentTimeHours < DuskStartHour)
	{
		return DayTimeColor;
	}

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

	return NightTimeColor;
}

void UTFDayNightWidget::OnTimeChanged(float CurrentTimeHours)
{
	UpdateTimeDisplay(CurrentTimeHours);
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
	if (CachedDayNightCycle)
	{
		CachedDayNightCycle->OnTimeChanged.RemoveAll(this);
		CachedDayNightCycle->OnDayChanged.RemoveAll(this);
		CachedDayNightCycle->OnDayNightStateChanged.RemoveAll(this);
	}

	CachedDayNightCycle = NewDayNightCycle;

	if (CachedDayNightCycle)
	{
		CachedDayNightCycle->OnTimeChanged.AddUObject(this, &UTFDayNightWidget::OnTimeChanged);
		CachedDayNightCycle->OnDayChanged.AddUObject(this, &UTFDayNightWidget::OnDayChanged);
		CachedDayNightCycle->OnDayNightStateChanged.AddUObject(this, &UTFDayNightWidget::OnDayNightStateChanged);

		UpdateTimeDisplay(CachedDayNightCycle->GetCurrentTimeHours());
		UpdateDayDisplay(CachedDayNightCycle->GetCurrentDay());
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
