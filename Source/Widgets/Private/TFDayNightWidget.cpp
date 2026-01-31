// Copyright TF Project. All Rights Reserved.

#include "TFDayNightWidget.h"
#include "TFDayNightCycle.h"
#include "TFGameMode.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"

void UTFDayNightWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeDayNightCycle();
}

void UTFDayNightWidget::NativeDestruct()
{
	// Unbind from day/night cycle to prevent crashes
	if (CachedDayNightCycle)
	{
		CachedDayNightCycle->OnTimeChanged.RemoveAll(this);
		CachedDayNightCycle->OnDayChanged.RemoveAll(this);
		CachedDayNightCycle->OnDayNightStateChanged.RemoveAll(this);
		CachedDayNightCycle = nullptr;
	}

	Super::NativeDestruct();
}

void UTFDayNightWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CachedDayNightCycle)
	{
		InitializeDayNightCycle();
	}
	// Time display is updated via OnTimeChanged delegate, no per-frame update needed
}

void UTFDayNightWidget::InitializeDayNightCycle()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Use GameMode cached reference instead of GetAllActorsOfClass
	if (ATFGameMode* GM = Cast<ATFGameMode>(World->GetAuthGameMode()))
	{
		CachedDayNightCycle = GM->GetDayNightCycle();
	}

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
