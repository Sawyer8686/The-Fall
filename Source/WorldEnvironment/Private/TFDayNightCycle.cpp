#include "TFDayNightCycle.h"
#include "Engine/DirectionalLight.h"
#include "Components/LightComponent.h"

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
    NightStartHour = 20.0f; // 6 PM
    bCycleActive = true;

    // Sun light defaults
    SunLight = nullptr;
    bControlSunLight = true;

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

    // Initialize sun light
    UpdateSunLight();
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

    // Update sun light
    UpdateSunLight();
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
    // Map time to rotation based on DayStartHour and NightStartHour
    // DayStartHour (sunrise) = 0 degrees (sun at horizon)
    // Midday = -90 degrees (sun at zenith)
    // NightStartHour (sunset) = -180 degrees (sun at opposite horizon)
    // Midnight = -270 degrees (sun below horizon)

    const float DayDuration = NightStartHour - DayStartHour;
    const float NightDuration = 24.0f - DayDuration;

    if (CurrentTimeHours >= DayStartHour && CurrentTimeHours < NightStartHour)
    {
        // Daytime: sun goes from 0° (sunrise) to -180° (sunset)
        const float Alpha = (CurrentTimeHours - DayStartHour) / DayDuration;
        return -Alpha * 180.0f;
    }
    else
    {
        // Nighttime: sun goes from -180° (sunset) to -360° (sunrise)
        float TimeIntoNight;
        if (CurrentTimeHours >= NightStartHour)
        {
            TimeIntoNight = CurrentTimeHours - NightStartHour;
        }
        else
        {
            TimeIntoNight = (24.0f - NightStartHour) + CurrentTimeHours;
        }
        const float Alpha = TimeIntoNight / NightDuration;
        return -180.0f - Alpha * 180.0f;
    }
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

void ATFDayNightCycle::SetSunLight(ADirectionalLight* NewSunLight)
{
    SunLight = NewSunLight;
    UpdateSunLight();
}

FLinearColor ATFDayNightCycle::GetCurrentLightColor() const
{
    const int32 Phase = GetDayPhase();
    const float Alpha = GetPhaseAlpha();

    switch (Phase)
    {
    case 0: // Night
        return NightLightColor;

    case 1: // Dawn (night -> day)
        return FMath::Lerp(NightLightColor, DawnLightColor, Alpha);

    case 2: // Morning (dawn -> day)
        return FMath::Lerp(DawnLightColor, DayLightColor, Alpha);

    case 3: // Day
        return DayLightColor;

    case 4: // Dusk (day -> dusk)
        return FMath::Lerp(DayLightColor, DuskLightColor, Alpha);

    case 5: // Evening (dusk -> night)
        return FMath::Lerp(DuskLightColor, NightLightColor, Alpha);

    default:
        return DayLightColor;
    }
}

float ATFDayNightCycle::GetCurrentLightIntensity() const
{
    const int32 Phase = GetDayPhase();
    const float Alpha = GetPhaseAlpha();

    switch (Phase)
    {
    case 0: // Night
        return NightLightIntensity;

    case 1: // Dawn (night -> day)
        return FMath::Lerp(NightLightIntensity, DayLightIntensity * 0.5f, Alpha);

    case 2: // Morning (dawn -> day)
        return FMath::Lerp(DayLightIntensity * 0.5f, DayLightIntensity, Alpha);

    case 3: // Day
        return DayLightIntensity;

    case 4: // Dusk (day -> dusk)
        return FMath::Lerp(DayLightIntensity, DayLightIntensity * 0.5f, Alpha);

    case 5: // Evening (dusk -> night)
        return FMath::Lerp(DayLightIntensity * 0.5f, NightLightIntensity, Alpha);

    default:
        return DayLightIntensity;
    }
}

void ATFDayNightCycle::RefreshSunLight()
{
    UpdateSunLight();
}

void ATFDayNightCycle::UpdateSunLight()
{
    if (!SunLight || !bControlSunLight)
    {
        return;
    }

    // Update rotation
    // Sun rises in the east, sets in the west
    // Pitch rotation: -90 at midnight (below horizon), 0 at sunrise, 90 at noon, 180 at sunset
    const float SunPitch = GetSunRotation();
    FRotator NewRotation = SunRotationAxis;
    NewRotation.Pitch = SunPitch;
    SunLight->SetActorRotation(NewRotation);

    // Update light color
    ULightComponent* LightComponent = SunLight->GetLightComponent();
    if (LightComponent)
    {
        LightComponent->SetLightColor(GetCurrentLightColor());
        LightComponent->SetIntensity(GetCurrentLightIntensity());
    }
}

int32 ATFDayNightCycle::GetDayPhase() const
{
    const float DawnEnd = DayStartHour + DawnDurationHours;
    const float DuskStart = NightStartHour - DuskDurationHours;

    // Phase 0: Night (before dawn)
    if (CurrentTimeHours < DayStartHour)
    {
        return 0;
    }

    // Phase 1: Dawn first half (night -> dawn color)
    if (CurrentTimeHours < DayStartHour + DawnDurationHours * 0.5f)
    {
        return 1;
    }

    // Phase 2: Dawn second half (dawn -> day color)
    if (CurrentTimeHours < DawnEnd)
    {
        return 2;
    }

    // Phase 3: Day
    if (CurrentTimeHours < DuskStart)
    {
        return 3;
    }

    // Phase 4: Dusk first half (day -> dusk color)
    if (CurrentTimeHours < DuskStart + DuskDurationHours * 0.5f)
    {
        return 4;
    }

    // Phase 5: Dusk second half (dusk -> night color)
    if (CurrentTimeHours < NightStartHour)
    {
        return 5;
    }

    // Phase 0: Night (after dusk)
    return 0;
}

float ATFDayNightCycle::GetPhaseAlpha() const
{
    const float DawnEnd = DayStartHour + DawnDurationHours;
    const float DuskStart = NightStartHour - DuskDurationHours;
    const float HalfDawn = DawnDurationHours * 0.5f;
    const float HalfDusk = DuskDurationHours * 0.5f;

    const int32 Phase = GetDayPhase();

    switch (Phase)
    {
    case 1: // Dawn first half
        return (CurrentTimeHours - DayStartHour) / HalfDawn;

    case 2: // Dawn second half
        return (CurrentTimeHours - (DayStartHour + HalfDawn)) / HalfDawn;

    case 4: // Dusk first half
        return (CurrentTimeHours - DuskStart) / HalfDusk;

    case 5: // Dusk second half
        return (CurrentTimeHours - (DuskStart + HalfDusk)) / HalfDusk;

    default:
        return 0.0f;
    }
}
