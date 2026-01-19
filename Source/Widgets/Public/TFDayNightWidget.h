// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TFDayNightWidget.generated.h"

class ATFDayNightCycle;
class UTextBlock;
class UImage;

/**
 * Day/Night HUD Widget
 * Displays current time and day information
 */
UCLASS()
class WIDGETS_API UTFDayNightWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

#pragma region Widget Bindings

	/** Text block displaying the current time (HH:MM) */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimeText;

	/** Optional text block displaying the current day */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* DayText;

	/** Optional image for day/night icon */
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* DayNightIcon;

	/** Optional text block displaying seconds (HH:MM:SS format) */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TimeWithSecondsText;

#pragma endregion Widget Bindings

#pragma region Visual Settings

	UPROPERTY(EditAnywhere, Category = "Day Night|Colors")
	FLinearColor DayTimeColor = FLinearColor(1.0f, 0.9f, 0.4f);

	UPROPERTY(EditAnywhere, Category = "Day Night|Colors")
	FLinearColor NightTimeColor = FLinearColor(0.4f, 0.5f, 0.9f);

	UPROPERTY(EditAnywhere, Category = "Day Night|Colors")
	FLinearColor TransitionColor = FLinearColor(1.0f, 0.6f, 0.3f);

	UPROPERTY(EditAnywhere, Category = "Day Night|Format")
	FString DayTextFormat = TEXT("Day {day}");

	UPROPERTY(EditAnywhere, Category = "Day Night|Transition", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float DawnStartHour = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Day Night|Transition", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float DawnEndHour = 7.0f;

	UPROPERTY(EditAnywhere, Category = "Day Night|Transition", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float DuskStartHour = 19.0f;

	UPROPERTY(EditAnywhere, Category = "Day Night|Transition", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float DuskEndHour = 21.0f;

#pragma endregion Visual Settings

#pragma region Icon Settings

	UPROPERTY(EditAnywhere, Category = "Day Night|Icons")
	UTexture2D* DayIconTexture;

	UPROPERTY(EditAnywhere, Category = "Day Night|Icons")
	UTexture2D* NightIconTexture;

#pragma endregion Icon Settings

private:

	/** Cached day night cycle reference */
	UPROPERTY()
	ATFDayNightCycle* CachedDayNightCycle;

	/** Last known day state for change detection */
	bool bLastWasDay = true;

protected:

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Find and cache the day night cycle actor in the world */
	void InitializeDayNightCycle();

	/** Update the time display */
	void UpdateTimeDisplay(float CurrentTimeHours);

	/** Update the day display */
	void UpdateDayDisplay(int32 CurrentDay);

	/** Update the color based on time of day */
	void UpdateTimeColor(float CurrentTimeHours);

	/** Update the day/night icon */
	void UpdateDayNightIcon(bool bIsDay);

	FLinearColor GetTimeColor(float CurrentTimeHours) const;

	void OnTimeChanged(float CurrentTimeHours);
	void OnDayChanged(int32 CurrentDay);
	void OnDayNightStateChanged(bool bIsDay);

public:

	void SetDayNightCycle(ATFDayNightCycle* NewDayNightCycle);
	FString GetCurrentTimeString() const;
	int32 GetCurrentDay() const;
	bool IsDay() const;
	ATFDayNightCycle* GetDayNightCycle() const { return CachedDayNightCycle; }
};
