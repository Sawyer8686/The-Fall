#include "TFDayNightCycle.h"

ATFDayNightCycle::ATFDayNightCycle()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default time settings
    CurrentTimeHours = 8.0f;  // Start at 8 AM
    CurrentDay = 1;
    StartingTimeHours = 8.0f;
    StartingDay = 1;

    // Default cycle settings
    RealSecondsPerGameHour = 60.0f;  // 1 real minute = 1 game hour (24 minutes for full day)
    DayStartHour = 6.0f;   // 6 AM
    NightStartHour = 20.0f; // 8 PM
    bCycleActive = true;

    bWasDay = true;
}

void ATFDayNightCycle::BeginPlay()
{
    Super::BeginPlay();

    // Initialize with starting values
    CurrentTimeHours = StartingTimeHours;
    CurrentDay = StartingDay;
    bWasDay = IsDay();

    // Broadcast initial state
    BroadcastTimeChanged();
    OnDayChanged.Broadcast(CurrentDay);
    OnDayNightStateChanged.Broadcast(bWasDay);
}

void ATFDayNightCycle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bCycleActive)
    {
        UpdateTime(DeltaTime);
    }
}

void ATFDayNightCycle::UpdateTime(float DeltaTime)
{
    // Calculate how many game hours pass per real second
    const float GameHoursPerRealSecond = 1.0f / RealSecondsPerGameHour;

    // Update time
    CurrentTimeHours += DeltaTime * GameHoursPerRealSecond;

    // Handle day rollover
    while (CurrentTimeHours >= 24.0f)
    {
        CurrentTimeHours -= 24.0f;
        CurrentDay++;
        OnDayChanged.Broadcast(CurrentDay);
    }

    // Check for day/night state change
    const bool bIsCurrentlyDay = IsDay();
    if (bIsCurrentlyDay != bWasDay)
    {
        bWasDay = bIsCurrentlyDay;
        OnDayNightStateChanged.Broadcast(bIsCurrentlyDay);
    }

    // Broadcast time update
    BroadcastTimeChanged();
}

void ATFDayNightCycle::BroadcastTimeChanged()
{
    OnTimeChanged.Broadcast(CurrentTimeHours);
}

FString ATFDayNightCycle::GetFormattedTime() const
{
    const int32 Hours = FMath::FloorToInt32(CurrentTimeHours);
    const int32 Minutes = FMath::FloorToInt32(FMath::Frac(CurrentTimeHours) * 60.0f);

    return FString::Printf(TEXT("%02d:%02d"), Hours, Minutes);
}

FString ATFDayNightCycle::GetFormattedTimeWithSeconds() const
{
    const int32 Hours = FMath::FloorToInt32(CurrentTimeHours);
    const float FractionalHours = FMath::Frac(CurrentTimeHours);
    const int32 Minutes = FMath::FloorToInt32(FractionalHours * 60.0f);
    const int32 Seconds = FMath::FloorToInt32(FMath::Frac(FractionalHours * 60.0f) * 60.0f);

    return FString::Printf(TEXT("%02d:%02d:%02d"), Hours, Minutes, Seconds);
}

bool ATFDayNightCycle::IsDay() const
{
    return CurrentTimeHours >= DayStartHour && CurrentTimeHours < NightStartHour;
}

float ATFDayNightCycle::GetSunRotation() const
{
    // Map 0-24 hours to 0-360 degrees
    // 0 (midnight) = -90 degrees (sun below horizon)
    // 6 (sunrise) = 0 degrees
    // 12 (noon) = 90 degrees (sun at zenith)
    // 18 (sunset) = 180 degrees
    // 24 (midnight) = 270 degrees
    return (CurrentTimeHours / 24.0f) * 360.0f - 90.0f;
}

void ATFDayNightCycle::SetTime(float NewTimeHours)
{
    CurrentTimeHours = FMath::Clamp(NewTimeHours, 0.0f, 23.999f);

    // Check for day/night state change
    const bool bIsCurrentlyDay = IsDay();
    if (bIsCurrentlyDay != bWasDay)
    {
        bWasDay = bIsCurrentlyDay;
        OnDayNightStateChanged.Broadcast(bIsCurrentlyDay);
    }

    BroadcastTimeChanged();
}

void ATFDayNightCycle::SetDay(int32 NewDay)
{
    if (NewDay >= 1)
    {
        CurrentDay = NewDay;
        OnDayChanged.Broadcast(CurrentDay);
    }
}

void ATFDayNightCycle::AddHours(float HoursToAdd)
{
    float NewTime = CurrentTimeHours + HoursToAdd;

    // Handle negative time
    while (NewTime < 0.0f)
    {
        NewTime += 24.0f;
        if (CurrentDay > 1)
        {
            CurrentDay--;
            OnDayChanged.Broadcast(CurrentDay);
        }
    }

    // Handle day rollover
    while (NewTime >= 24.0f)
    {
        NewTime -= 24.0f;
        CurrentDay++;
        OnDayChanged.Broadcast(CurrentDay);
    }

    CurrentTimeHours = NewTime;

    // Check for day/night state change
    const bool bIsCurrentlyDay = IsDay();
    if (bIsCurrentlyDay != bWasDay)
    {
        bWasDay = bIsCurrentlyDay;
        OnDayNightStateChanged.Broadcast(bIsCurrentlyDay);
    }

    BroadcastTimeChanged();
}

void ATFDayNightCycle::SkipToSunrise()
{
    if (CurrentTimeHours >= DayStartHour)
    {
        // Already past sunrise, skip to next day's sunrise
        CurrentDay++;
        OnDayChanged.Broadcast(CurrentDay);
    }

    SetTime(DayStartHour);
}

void ATFDayNightCycle::SkipToSunset()
{
    if (CurrentTimeHours >= NightStartHour)
    {
        // Already past sunset, skip to next day's sunset
        CurrentDay++;
        OnDayChanged.Broadcast(CurrentDay);
    }

    SetTime(NightStartHour);
}

void ATFDayNightCycle::SetCycleSpeed(float NewRealSecondsPerGameHour)
{
    RealSecondsPerGameHour = FMath::Max(0.1f, NewRealSecondsPerGameHour);
}
