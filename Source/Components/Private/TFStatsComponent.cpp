// Fill out your copyright notice in the Description page of Project Settings.

#include "TFStatsComponent.h"
#include "TimerManager.h"

UTFStatsComponent::UTFStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTFStatsComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize stats to max
	CurrentHunger = MaxHunger;
	CurrentThirst = MaxThirst;

	// Start decay timers
	StartDecayTimers();
}

void UTFStatsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopDecayTimers();

	Super::EndPlay(EndPlayReason);
}

void UTFStatsComponent::StartDecayTimers()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Start hunger decay timer
	World->GetTimerManager().SetTimer(
		HungerDecayTimerHandle,
		this,
		&UTFStatsComponent::DecayHunger,
		HungerDecayInterval,
		true
	);

	// Start thirst decay timer
	World->GetTimerManager().SetTimer(
		ThirstDecayTimerHandle,
		this,
		&UTFStatsComponent::DecayThirst,
		ThirstDecayInterval,
		true
	);
}

void UTFStatsComponent::StopDecayTimers()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(HungerDecayTimerHandle);
	World->GetTimerManager().ClearTimer(ThirstDecayTimerHandle);
}

void UTFStatsComponent::DecayHunger()
{
	ConsumeHunger(HungerDecayAmount);
}

void UTFStatsComponent::DecayThirst()
{
	ConsumeThirst(ThirstDecayAmount);
}

void UTFStatsComponent::UpdateHungerCriticalState()
{
	float HungerPercent = GetHungerPercent();
	bool bWasCritical = bIsHungerCritical;

	bIsHungerCritical = HungerPercent <= HungerCriticalThreshold && HungerPercent > 0.0f;

	// Broadcast critical event when entering critical state
	if (bIsHungerCritical && !bWasCritical)
	{
		OnStatCritical.Broadcast(FName("Hunger"), HungerPercent);
	}

	// Check for depletion
	bool bWasDepleted = bIsHungerDepleted;
	bIsHungerDepleted = CurrentHunger <= 0.0f;

	if (bIsHungerDepleted && !bWasDepleted)
	{
		OnStatDepleted.Broadcast(FName("Hunger"));
	}
}

void UTFStatsComponent::UpdateThirstCriticalState()
{
	float ThirstPercent = GetThirstPercent();
	bool bWasCritical = bIsThirstCritical;

	bIsThirstCritical = ThirstPercent <= ThirstCriticalThreshold && ThirstPercent > 0.0f;

	// Broadcast critical event when entering critical state
	if (bIsThirstCritical && !bWasCritical)
	{
		OnStatCritical.Broadcast(FName("Thirst"), ThirstPercent);
	}

	// Check for depletion
	bool bWasDepleted = bIsThirstDepleted;
	bIsThirstDepleted = CurrentThirst <= 0.0f;

	if (bIsThirstDepleted && !bWasDepleted)
	{
		OnStatDepleted.Broadcast(FName("Thirst"));
	}
}

#pragma region Hunger Functions

void UTFStatsComponent::ConsumeHunger(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	float OldHunger = CurrentHunger;
	CurrentHunger = FMath::Clamp(CurrentHunger - Amount, 0.0f, MaxHunger);

	if (OldHunger != CurrentHunger)
	{
		OnHungerChanged.Broadcast(CurrentHunger, MaxHunger);
		UpdateHungerCriticalState();
	}
}

void UTFStatsComponent::RestoreHunger(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	float OldHunger = CurrentHunger;
	CurrentHunger = FMath::Clamp(CurrentHunger + Amount, 0.0f, MaxHunger);

	if (OldHunger != CurrentHunger)
	{
		OnHungerChanged.Broadcast(CurrentHunger, MaxHunger);
		UpdateHungerCriticalState();
	}
}

void UTFStatsComponent::SetHunger(float NewHunger)
{
	float OldHunger = CurrentHunger;
	CurrentHunger = FMath::Clamp(NewHunger, 0.0f, MaxHunger);

	if (OldHunger != CurrentHunger)
	{
		OnHungerChanged.Broadcast(CurrentHunger, MaxHunger);
		UpdateHungerCriticalState();
	}
}

void UTFStatsComponent::FullyRestoreHunger()
{
	SetHunger(MaxHunger);
}

#pragma endregion Hunger Functions

#pragma region Thirst Functions

void UTFStatsComponent::ConsumeThirst(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	float OldThirst = CurrentThirst;
	CurrentThirst = FMath::Clamp(CurrentThirst - Amount, 0.0f, MaxThirst);

	if (OldThirst != CurrentThirst)
	{
		OnThirstChanged.Broadcast(CurrentThirst, MaxThirst);
		UpdateThirstCriticalState();
	}
}

void UTFStatsComponent::RestoreThirst(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	float OldThirst = CurrentThirst;
	CurrentThirst = FMath::Clamp(CurrentThirst + Amount, 0.0f, MaxThirst);

	if (OldThirst != CurrentThirst)
	{
		OnThirstChanged.Broadcast(CurrentThirst, MaxThirst);
		UpdateThirstCriticalState();
	}
}

void UTFStatsComponent::SetThirst(float NewThirst)
{
	float OldThirst = CurrentThirst;
	CurrentThirst = FMath::Clamp(NewThirst, 0.0f, MaxThirst);

	if (OldThirst != CurrentThirst)
	{
		OnThirstChanged.Broadcast(CurrentThirst, MaxThirst);
		UpdateThirstCriticalState();
	}
}

void UTFStatsComponent::FullyRestoreThirst()
{
	SetThirst(MaxThirst);
}

#pragma endregion Thirst Functions

#pragma region Queries

float UTFStatsComponent::GetHungerPercent() const
{
	return MaxHunger > 0.0f ? (CurrentHunger / MaxHunger) : 0.0f;
}

float UTFStatsComponent::GetThirstPercent() const
{
	return MaxThirst > 0.0f ? (CurrentThirst / MaxThirst) : 0.0f;
}

#pragma endregion Queries

#pragma region Configuration

void UTFStatsComponent::SetMaxHunger(float NewMax)
{
	MaxHunger = FMath::Max(1.0f, NewMax);
	CurrentHunger = FMath::Min(CurrentHunger, MaxHunger);
	OnHungerChanged.Broadcast(CurrentHunger, MaxHunger);
}

void UTFStatsComponent::SetHungerDecayRate(float DecayAmount, float DecayInterval)
{
	HungerDecayAmount = FMath::Max(0.0f, DecayAmount);
	HungerDecayInterval = FMath::Max(0.1f, DecayInterval);

	// Restart hunger timer with new interval
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(HungerDecayTimerHandle);
		World->GetTimerManager().SetTimer(
			HungerDecayTimerHandle,
			this,
			&UTFStatsComponent::DecayHunger,
			HungerDecayInterval,
			true
		);
	}
}

void UTFStatsComponent::SetMaxThirst(float NewMax)
{
	MaxThirst = FMath::Max(1.0f, NewMax);
	CurrentThirst = FMath::Min(CurrentThirst, MaxThirst);
	OnThirstChanged.Broadcast(CurrentThirst, MaxThirst);
}

void UTFStatsComponent::SetThirstDecayRate(float DecayAmount, float DecayInterval)
{
	ThirstDecayAmount = FMath::Max(0.0f, DecayAmount);
	ThirstDecayInterval = FMath::Max(0.1f, DecayInterval);

	// Restart thirst timer with new interval
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(ThirstDecayTimerHandle);
		World->GetTimerManager().SetTimer(
			ThirstDecayTimerHandle,
			this,
			&UTFStatsComponent::DecayThirst,
			ThirstDecayInterval,
			true
		);
	}
}

void UTFStatsComponent::SetDecayPaused(bool bPaused)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (bPaused)
	{
		World->GetTimerManager().PauseTimer(HungerDecayTimerHandle);
		World->GetTimerManager().PauseTimer(ThirstDecayTimerHandle);
	}
	else
	{
		World->GetTimerManager().UnPauseTimer(HungerDecayTimerHandle);
		World->GetTimerManager().UnPauseTimer(ThirstDecayTimerHandle);
	}
}

#pragma endregion Configuration
