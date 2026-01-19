#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TFDayNightCycle.generated.h"

class ADirectionalLight;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTimeChanged, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDayChanged, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDayNightStateChanged, bool);

UCLASS()
class WORLDENVIRONMENT_API ATFDayNightCycle : public AActor
{
    GENERATED_BODY()

public:
    ATFDayNightCycle();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

#pragma region Time Properties

    UPROPERTY(EditAnywhere, Category = "Day Night Cycle|Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeHours;

    UPROPERTY(EditAnywhere, Category = "Day Night Cycle|Time", meta = (ClampMin = "1"))
    int32 CurrentDay;

    UPROPERTY(EditAnywhere, Category = "Day Night Cycle|Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float StartingTimeHours;

    UPROPERTY(EditAnywhere, Category = "Day Night Cycle|Time", meta = (ClampMin = "1"))
    int32 StartingDay;

#pragma endregion

#pragma region Cycle Settings

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Settings", meta = (ClampMin = "0.1"))
    float RealSecondsPerGameHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Settings", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float DayStartHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Settings", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float NightStartHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Settings")
    bool bCycleActive;

#pragma endregion

#pragma region Sun Light Settings

    UPROPERTY(EditAnywhere, Category = "Day Night Cycle|Sun Light")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, Category = "Day Night Cycle|Sun Light")
    bool bControlSunLight = true;

    UPROPERTY(EditAnywhere, Category = "Day Night Cycle|Sun Light")
    FRotator SunRotationAxis = FRotator(0.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, Category = "Day Night Cycle|Sun Light|Colors")
    FLinearColor DawnLightColor = FLinearColor(1.0f, 0.6f, 0.3f);

    UPROPERTY(EditAnywhere, Category = "Day Night Cycle|Sun Light|Colors")
    FLinearColor DayLightColor = FLinearColor(1.0f, 0.98f, 0.95f);

    UPROPERTY(EditAnywhere, Category = "Day Night Cycle|Sun Light|Colors")
    FLinearColor DuskLightColor = FLinearColor(1.0f, 0.5f, 0.2f);

    UPROPERTY(EditAnywhere, Category = "Day Night Cycle|Sun Light|Colors")
    FLinearColor NightLightColor = FLinearColor(0.2f, 0.3f, 0.5f);

    UPROPERTY(EditAnywhere, Category = "Day Night Cycle|Sun Light|Intensity", meta = (ClampMin = "0.0"))
    float DayLightIntensity = 10.0f;

    UPROPERTY(EditAnywhere, Category = "Day Night Cycle|Sun Light|Intensity", meta = (ClampMin = "0.0"))
    float NightLightIntensity = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Day Night Cycle|Sun Light|Transition", meta = (ClampMin = "0.1", ClampMax = "6.0"))
    float DawnDurationHours = 1.5f;

    UPROPERTY(EditAnywhere, Category = "Day Night Cycle|Sun Light|Transition", meta = (ClampMin = "0.1", ClampMax = "6.0"))
    float DuskDurationHours = 1.5f;

#pragma endregion

#pragma region Delegates

    FOnTimeChanged OnTimeChanged;
    FOnDayChanged OnDayChanged;
    FOnDayNightStateChanged OnDayNightStateChanged;

#pragma endregion

#pragma region Functions

    float GetCurrentTimeHours() const { return CurrentTimeHours; }
    FString GetFormattedTime() const;
    FString GetFormattedTimeWithSeconds() const;
    int32 GetCurrentDay() const { return CurrentDay; }
    bool IsDay() const;
    bool IsNight() const { return !IsDay(); }
    float GetSunRotation() const;
    float GetNormalizedTimeOfDay() const { return CurrentTimeHours / 24.0f; }
    void SetTime(float NewTimeHours);
    void SetDay(int32 NewDay);
    void AddHours(float HoursToAdd);
    void SkipToSunrise();
    void SkipToSunset();
    void PauseCycle() { bCycleActive = false; }
    void ResumeCycle() { bCycleActive = true; }
    void ToggleCycle() { bCycleActive = !bCycleActive; }
    void SetCycleSpeed(float NewRealSecondsPerGameHour);
    void SetSunLight(ADirectionalLight* NewSunLight);
    FLinearColor GetCurrentLightColor() const;
    float GetCurrentLightIntensity() const;
    void RefreshSunLight();

#pragma endregion

private:
    /** Tracks whether it was day in the previous tick for state change detection */
    bool bWasDay;

    /** Update the time based on delta time */
    void UpdateTime(float DeltaTime);

    /** Broadcast time changed event */
    void BroadcastTimeChanged();

    /** Update sun light rotation, color, and intensity */
    void UpdateSunLight();

    /** Calculate the current phase of the day (0=night, 1=dawn, 2=day, 3=dusk) */
    int32 GetDayPhase() const;

    /** Get interpolation alpha for current phase transition */
    float GetPhaseAlpha() const;
};
