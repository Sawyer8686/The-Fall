#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TFDayNightCycle.generated.h"

class ADirectionalLight;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeChanged, float, CurrentTimeHours);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDayChanged, int32, CurrentDay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDayNightStateChanged, bool, bIsDay);

/**
 * Actor that manages the day/night cycle and time system.
 * Simulates the passage of time with configurable speed.
 */
UCLASS(Blueprintable, BlueprintType)
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

    /** Current time in hours (0-24). 0 = midnight, 12 = noon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeHours;

    /** Current day number (starts at 1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Time", meta = (ClampMin = "1"))
    int32 CurrentDay;

    /** Starting time when the game begins (in hours, 0-24) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float StartingTimeHours;

    /** Starting day when the game begins */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Time", meta = (ClampMin = "1"))
    int32 StartingDay;

#pragma endregion

#pragma region Cycle Settings

    /** How many real-world seconds equals one in-game hour. Lower = faster time. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Settings", meta = (ClampMin = "0.1"))
    float RealSecondsPerGameHour;

    /** Hour when day starts (sunrise). Default: 6:00 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Settings", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float DayStartHour;

    /** Hour when night starts (sunset). Default: 20:00 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Settings", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float NightStartHour;

    /** Whether the time cycle is currently active */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Settings")
    bool bCycleActive;

#pragma endregion

#pragma region Sun Light Settings

    /** Reference to the directional light acting as the sun */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Sun Light")
    ADirectionalLight* SunLight;

    /** Enable automatic sun light control */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Sun Light")
    bool bControlSunLight = true;

    /** Rotation axis for the sun (typically pitch for east-west movement) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Sun Light")
    FRotator SunRotationAxis = FRotator(0.0f, 0.0f, 0.0f);

    /** Sun light color at dawn */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Sun Light|Colors")
    FLinearColor DawnLightColor = FLinearColor(1.0f, 0.6f, 0.3f);

    /** Sun light color at noon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Sun Light|Colors")
    FLinearColor DayLightColor = FLinearColor(1.0f, 0.98f, 0.95f);

    /** Sun light color at dusk */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Sun Light|Colors")
    FLinearColor DuskLightColor = FLinearColor(1.0f, 0.5f, 0.2f);

    /** Moon light color at night */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Sun Light|Colors")
    FLinearColor NightLightColor = FLinearColor(0.2f, 0.3f, 0.5f);

    /** Sun light intensity during the day */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Sun Light|Intensity", meta = (ClampMin = "0.0"))
    float DayLightIntensity = 10.0f;

    /** Moon light intensity during the night */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Sun Light|Intensity", meta = (ClampMin = "0.0"))
    float NightLightIntensity = 0.5f;

    /** Duration of dawn transition in hours */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Sun Light|Transition", meta = (ClampMin = "0.1", ClampMax = "6.0"))
    float DawnDurationHours = 1.5f;

    /** Duration of dusk transition in hours */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Sun Light|Transition", meta = (ClampMin = "0.1", ClampMax = "6.0"))
    float DuskDurationHours = 1.5f;

#pragma endregion

#pragma region Delegates

    /** Broadcast every time update with the current time in hours */
    UPROPERTY(BlueprintAssignable, Category = "Day Night Cycle|Events")
    FOnTimeChanged OnTimeChanged;

    /** Broadcast when the day number changes */
    UPROPERTY(BlueprintAssignable, Category = "Day Night Cycle|Events")
    FOnDayChanged OnDayChanged;

    /** Broadcast when transitioning between day and night */
    UPROPERTY(BlueprintAssignable, Category = "Day Night Cycle|Events")
    FOnDayNightStateChanged OnDayNightStateChanged;

#pragma endregion

#pragma region Blueprint Functions

    /** Get the current time as hours (0-24) */
    UFUNCTION(BlueprintPure, Category = "Day Night Cycle")
    float GetCurrentTimeHours() const { return CurrentTimeHours; }

    /** Get the current time as a formatted string (HH:MM) */
    UFUNCTION(BlueprintPure, Category = "Day Night Cycle")
    FString GetFormattedTime() const;

    /** Get the current time as a formatted string with seconds (HH:MM:SS) */
    UFUNCTION(BlueprintPure, Category = "Day Night Cycle")
    FString GetFormattedTimeWithSeconds() const;

    /** Get the current day number */
    UFUNCTION(BlueprintPure, Category = "Day Night Cycle")
    int32 GetCurrentDay() const { return CurrentDay; }

    /** Check if it's currently daytime */
    UFUNCTION(BlueprintPure, Category = "Day Night Cycle")
    bool IsDay() const;

    /** Check if it's currently nighttime */
    UFUNCTION(BlueprintPure, Category = "Day Night Cycle")
    bool IsNight() const { return !IsDay(); }

    /** Get the sun/moon rotation based on current time (0-360 degrees) */
    UFUNCTION(BlueprintPure, Category = "Day Night Cycle")
    float GetSunRotation() const;

    /** Get normalized time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight) */
    UFUNCTION(BlueprintPure, Category = "Day Night Cycle")
    float GetNormalizedTimeOfDay() const { return CurrentTimeHours / 24.0f; }

    /** Set the current time directly (in hours, 0-24) */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTime(float NewTimeHours);

    /** Set the current day directly */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetDay(int32 NewDay);

    /** Add hours to the current time (can be negative) */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void AddHours(float HoursToAdd);

    /** Skip to the next sunrise */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SkipToSunrise();

    /** Skip to the next sunset */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SkipToSunset();

    /** Pause the time cycle */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void PauseCycle() { bCycleActive = false; }

    /** Resume the time cycle */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void ResumeCycle() { bCycleActive = true; }

    /** Toggle the time cycle on/off */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void ToggleCycle() { bCycleActive = !bCycleActive; }

    /** Set the cycle speed (real seconds per game hour) */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetCycleSpeed(float NewRealSecondsPerGameHour);

    /** Set the sun light reference */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle|Sun Light")
    void SetSunLight(ADirectionalLight* NewSunLight);

    /** Get the current light color based on time of day */
    UFUNCTION(BlueprintPure, Category = "Day Night Cycle|Sun Light")
    FLinearColor GetCurrentLightColor() const;

    /** Get the current light intensity based on time of day */
    UFUNCTION(BlueprintPure, Category = "Day Night Cycle|Sun Light")
    float GetCurrentLightIntensity() const;

    /** Manually refresh the sun light (useful after changing properties) */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle|Sun Light")
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
