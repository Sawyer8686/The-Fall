// Copyright TF Project. All Rights Reserved.

#include "TFStaminaComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/TimerHandle.h"

UTFStaminaComponent::UTFStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UTFStaminaComponent::BeginPlay()
{
	Super::BeginPlay();

	EnsureTickEnabledIfNeeded();

	// Cache owner character reference
	OwnerCharacter = Cast<ACharacter>(GetOwner());

	// Initialize stamina to max
	CurrentStamina = MaxStamina;

	// Broadcast initial values
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

void UTFStaminaComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clear drain timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DrainTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UTFStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update regeneration delay timer
	if (RegenDelayTimer > 0.0f)
	{
		RegenDelayTimer -= DeltaTime;
		bIsRegenerating = false;
	}
	else
	{
		bIsRegenerating = true;
	}

	// Regenerate stamina if allowed
	if (bIsRegenerating && !bIsDraining && CurrentStamina < MaxStamina)
	{
		RegenerateStamina(DeltaTime);
	}

	// Update exhaustion state
	UpdateExhaustionState();

	// Disable tick if stamina is full and not draining
	if (CurrentStamina >= MaxStamina && !bIsDraining && !bIsExhausted)
	{
		SetComponentTickEnabled(false);
	}
}

void UTFStaminaComponent::RegenerateStamina(float DeltaTime)
{
	if (CurrentStamina >= MaxStamina)
	{
		return;
	}

	// Calculate regeneration rate based on character state and modifiers
	float RegenRate = GetCurrentRegenRate() * RegenRateMultiplier;

	// Apply exhaustion penalty
	if (bIsExhausted)
	{
		RegenRate *= ExhaustedRegenMultiplier;
	}

	// Regenerate stamina
	float OldStamina = CurrentStamina;
	CurrentStamina = FMath::Clamp(CurrentStamina + (RegenRate * DeltaTime), 0.0f, MaxStamina);

	// Broadcast change if stamina actually changed
	if (!FMath::IsNearlyEqual(OldStamina, CurrentStamina, 0.01f))
	{
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}
}

void UTFStaminaComponent::UpdateExhaustionState()
{
	float ExhaustionStaminaValue = MaxStamina * ExhaustionThreshold;
	float RecoveryStaminaValue = MaxStamina * ExhaustionRecoveryThreshold;

	// Check if should be exhausted
	if (CurrentStamina <= ExhaustionStaminaValue && !bIsExhausted)
	{
		bIsExhausted = true;
		OnStaminaDepleted.Broadcast();
	}
	// Check if recovered from exhaustion (using configurable hysteresis)
	else if (CurrentStamina > RecoveryStaminaValue && bIsExhausted)
	{
		bIsExhausted = false;
		OnStaminaRecovered.Broadcast();
	}
}

float UTFStaminaComponent::GetCurrentRegenRate() const
{
	if (!OwnerCharacter)
	{
		return StaminaRegenRate;
	}

	// Check if character is moving
	UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
	if (MovementComp && MovementComp->Velocity.SizeSquared() > 1.0f)
	{
		// Moving but not sprinting
		return StaminaRegenRateMoving;
	}

	// Idle - full regen rate
	return StaminaRegenRate;
}

bool UTFStaminaComponent::ConsumeStamina(float Amount, EStaminaDrainReason Reason)
{
	// Validate input
	if (Amount <= 0.0f)
	{
		return false;
	}

	// Check if we have enough stamina
	if (CurrentStamina < Amount)
	{
		return false;
	}

	// Consume stamina
	float OldStamina = CurrentStamina;
	CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.0f, MaxStamina);

	// Check if stamina was depleted
	bool bWasDepleted = (CurrentStamina <= 0.0f && OldStamina > 0.0f);

	// Reset regeneration delay
	ResetRegenDelay(bWasDepleted);

	// Enable tick for regeneration
	SetComponentTickEnabled(true);

	// Broadcast change
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);

	return true;
}

void UTFStaminaComponent::StartStaminaDrain(float DrainRate)
{
	if (bIsDraining || DrainRate <= 0.0f)
	{
		return;
	}

	bIsDraining = true;

	// Enable tick for exhaustion state updates
	SetComponentTickEnabled(true);

	constexpr float TimerTickRate = 0.1f;

	FTimerDelegate TimerDel;
	TimerDel.BindLambda([this, DrainRate, TimerTickRate]()
		{
			if (bIsDraining)
			{
				// Apply drain rate multiplier
				float EffectiveDrain = DrainRate * DrainRateMultiplier * TimerTickRate;

				float OldStamina = CurrentStamina;
				CurrentStamina = FMath::Clamp(CurrentStamina - EffectiveDrain, 0.0f, MaxStamina);

				// Check if depleted
				if (CurrentStamina <= 0.0f && OldStamina > 0.0f)
				{
					ResetRegenDelay(true);
					StopStaminaDrain();
				}

				OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
			}
		});

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			DrainTimerHandle,
			TimerDel,
			TimerTickRate,
			true
		);
	}
}

void UTFStaminaComponent::StopStaminaDrain()
{
	if (!bIsDraining)
	{
		return;
	}

	bIsDraining = false;

	// Clear the drain timer to stop the looping callback
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DrainTimerHandle);
	}

	// Reset regeneration delay after usage, but only if stamina wasn't depleted
	// (depletion already set the longer delay in StartStaminaDrain)
	if (CurrentStamina > 0.0f)
	{
		ResetRegenDelay(false);
	}
}

void UTFStaminaComponent::RestoreStamina(float Amount)
{
	float OldStamina = CurrentStamina;
	CurrentStamina = FMath::Clamp(CurrentStamina + Amount, 0.0f, MaxStamina);

	// Broadcast change if stamina actually changed
	if (!FMath::IsNearlyEqual(OldStamina, CurrentStamina, 0.01f))
	{
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}

	EnsureTickEnabledIfNeeded();
}

void UTFStaminaComponent::SetStamina(float NewStamina)
{
	CurrentStamina = FMath::Clamp(NewStamina, 0.0f, MaxStamina);
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);

	EnsureTickEnabledIfNeeded();
}

void UTFStaminaComponent::FullyRestoreStamina()
{
	CurrentStamina = MaxStamina;
	bIsExhausted = false;
	RegenDelayTimer = 0.0f;
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	OnStaminaRecovered.Broadcast();
}

void UTFStaminaComponent::ResetRegenDelay(bool bFromDepletion)
{
	RegenDelayTimer = bFromDepletion ? RegenDelayAfterDepletion : RegenDelayAfterUsage;
	bIsRegenerating = false;
}

bool UTFStaminaComponent::HasEnoughStamina(float Required) const
{
	return CurrentStamina >= Required;
}

bool UTFStaminaComponent::CanSprint() const
{
	return CurrentStamina >= MinStaminaToSprint && !bIsExhausted;
}

bool UTFStaminaComponent::CanJump() const
{
	return CurrentStamina >= MinStaminaToJump;
}

float UTFStaminaComponent::GetStaminaPercent() const
{
	return MaxStamina > 0.0f ? (CurrentStamina / MaxStamina) : 0.0f;
}

float UTFStaminaComponent::GetBreathingIntensity() const
{
	if (!bEnableBreathingEffects)
	{
		return 0.0f;
	}

	float StaminaPercent = GetStaminaPercent();

	// No breathing effect above threshold
	if (StaminaPercent > BreathingEffectThreshold)
	{
		return 0.0f;
	}

	// Calculate intensity (inverse of stamina percentage below threshold)
	float NormalizedIntensity = 1.0f - (StaminaPercent / BreathingEffectThreshold);
	return NormalizedIntensity * BreathingIntensity;
}

void UTFStaminaComponent::SetMaxStamina(float NewMax)
{
	if (NewMax <= 0.0f)
	{
		return;
	}

	// Scale current stamina proportionally
	float Percentage = GetStaminaPercent();
	MaxStamina = NewMax;
	CurrentStamina = MaxStamina * Percentage;

	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);

	EnsureTickEnabledIfNeeded();
}

void UTFStaminaComponent::SetRegenRate(float NewRate)
{
	StaminaRegenRate = FMath::Max(0.0f, NewRate);
}

void UTFStaminaComponent::SetDrainRateMultiplier(float Multiplier)
{
	DrainRateMultiplier = FMath::Max(0.0f, Multiplier);
}

void UTFStaminaComponent::SetRegenRateMultiplier(float Multiplier)
{
	RegenRateMultiplier = FMath::Max(0.0f, Multiplier);
}

void UTFStaminaComponent::ResetModifiers()
{
	DrainRateMultiplier = 1.0f;
	RegenRateMultiplier = 1.0f;
}

float UTFStaminaComponent::GetEffectiveDrainRate(float BaseDrainRate) const
{
	return BaseDrainRate * DrainRateMultiplier;
}

float UTFStaminaComponent::GetEffectiveRegenRate() const
{
	return GetCurrentRegenRate() * RegenRateMultiplier;
}

void UTFStaminaComponent::EnsureTickEnabledIfNeeded()
{
	if (CurrentStamina < MaxStamina || bIsDraining || bIsExhausted || RegenDelayTimer > 0.0f)
	{
		SetComponentTickEnabled(true);
	}
}
