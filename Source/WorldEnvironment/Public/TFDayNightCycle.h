#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TFDayNightCycle.generated.h"

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

#pragma endregion

private:
    /** Tracks whether it was day in the previous tick for state change detection */
    bool bWasDay;

    /** Update the time based on delta time */
    void UpdateTime(float DeltaTime);

    /** Broadcast time changed event */
    void BroadcastTimeChanged();
};
