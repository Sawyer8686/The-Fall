// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TFDayNightWidget.generated.h"

class ATFDayNightCycle;
class UTextBlock;
class UImage;

UCLASS()
class WIDGETS_API UTFDayNightWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

#pragma region Widget Bindings

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimeText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* DayText;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* DayNightIcon;

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

	UPROPERTY()
	ATFDayNightCycle* CachedDayNightCycle;

	bool bLastWasDay = true;

protected:

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	void InitializeDayNightCycle();
	void UpdateTimeDisplay(float CurrentTimeHours);
	void UpdateDayDisplay(int32 CurrentDay);
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
