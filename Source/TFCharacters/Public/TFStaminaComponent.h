// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TFStaminaComponent.generated.h"

class ATFCharacterBase;

/** Delegate for stamina changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float, CurrentStamina, float, MaxStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaDepleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaRecovered);

/** Stamina drain reasons for different costs */
UENUM(BlueprintType)
enum class EStaminaDrainReason : uint8
{
	Sprint		UMETA(DisplayName = "Sprint"),
	Jump		UMETA(DisplayName = "Jump"),
	Dodge		UMETA(DisplayName = "Dodge"),
	Attack		UMETA(DisplayName = "Attack"),
	Custom		UMETA(DisplayName = "Custom")
};

/**
 * Stamina Component - Manages character stamina system
 * Features:
 * - Automatic regeneration with configurable delays
 * - Different drain rates for various actions
 * - Exhaustion system with recovery penalty
 * - Visual feedback support (breathing, vignette)
 * - Network replication ready
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TFCHARACTERS_API UTFStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

private:

#pragma region Stamina Values

	/** Current stamina amount */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentStamina, VisibleAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true"))
	float CurrentStamina;

	/** Maximum stamina capacity */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true", ClampMin = "1.0", ClampMax = "1000.0"))
	float MaxStamina = 100.0f;

	/** Stamina percentage threshold for exhaustion state (0.0 - 1.0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Exhaustion", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float ExhaustionThreshold = 0.2f;

#pragma endregion Stamina Values

#pragma region Regeneration

	/** Stamina regeneration per second when idle */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Regeneration", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float StaminaRegenRate = 20.0f;

	/** Stamina regeneration per second when moving (not sprinting) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Regeneration", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float StaminaRegenRateMoving = 12.0f;

	/** Delay before stamina starts regenerating after depletion (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Regeneration", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float RegenDelayAfterDepletion = 2.0f;

	/** Delay before stamina starts regenerating after usage (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Regeneration", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float RegenDelayAfterUsage = 0.5f;

	/** Multiplier for regeneration when exhausted */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Exhaustion", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "2.0"))
	float ExhaustedRegenMultiplier = 0.5f;

#pragma endregion Regeneration

#pragma region Drain Costs

	/** Stamina drain per second while sprinting */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Drain", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float SprintDrainRate = 15.0f;

	/** Stamina cost for jumping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Drain", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float JumpStaminaCost = 10.0f;

	/** Stamina cost for dodging */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Drain", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float DodgeStaminaCost = 25.0f;

	/** Minimum stamina required to sprint */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Drain", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MinStaminaToSprint = 5.0f;

	/** Minimum stamina required to jump */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Drain", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MinStaminaToJump = 5.0f;

#pragma endregion Drain Costs

#pragma region State

	/** Is stamina currently regenerating */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina|State", meta = (AllowPrivateAccess = "true"))
	bool bIsRegenerating = true;

	/** Is character currently exhausted */
	UPROPERTY(ReplicatedUsing = OnRep_IsExhausted, VisibleAnywhere, BlueprintReadOnly, Category = "Stamina|State", meta = (AllowPrivateAccess = "true"))
	bool bIsExhausted = false;

	/** Is stamina currently being drained */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina|State", meta = (AllowPrivateAccess = "true"))
	bool bIsDraining = false;

	/** Timer for regeneration delay */
	float RegenDelayTimer = 0.0f;

	/** Cached owner character reference */
	UPROPERTY()
	ATFCharacterBase* OwnerCharacter;

#pragma endregion State

#pragma region Visual Feedback

	/** Enable breathing effects when low stamina */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Visual Feedback", meta = (AllowPrivateAccess = "true"))
	bool bEnableBreathingEffects = true;

	/** Stamina threshold for breathing effects (0.0 - 1.0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Visual Feedback", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float BreathingEffectThreshold = 0.3f;

	/** Breathing effect intensity multiplier */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Visual Feedback", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "5.0"))
	float BreathingIntensity = 1.0f;

#pragma endregion Visual Feedback

protected:

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Handle stamina regeneration logic */
	void RegenerateStamina(float DeltaTime);

	/** Check and update exhaustion state */
	void UpdateExhaustionState();

	/** Calculate current regeneration rate based on character state */
	float GetCurrentRegenRate() const;

	/** Replication callbacks */
	UFUNCTION()
	void OnRep_CurrentStamina();

	UFUNCTION()
	void OnRep_IsExhausted();

public:

	UTFStaminaComponent();

#pragma region Delegates

	/** Called when stamina value changes */
	UPROPERTY(BlueprintAssignable, Category = "Stamina|Events")
	FOnStaminaChanged OnStaminaChanged;

	/** Called when stamina is completely depleted */
	UPROPERTY(BlueprintAssignable, Category = "Stamina|Events")
	FOnStaminaDepleted OnStaminaDepleted;

	/** Called when stamina recovers from exhaustion */
	UPROPERTY(BlueprintAssignable, Category = "Stamina|Events")
	FOnStaminaRecovered OnStaminaRecovered;

#pragma endregion Delegates

#pragma region Core Functions

	/**
	 * Consume stamina for an action
	 * @param Amount - Amount of stamina to consume
	 * @param Reason - Reason for stamina consumption (for tracking/debugging)
	 * @return True if stamina was successfully consumed
	 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	bool ConsumeStamina(float Amount, EStaminaDrainReason Reason = EStaminaDrainReason::Custom);

	/**
	 * Start continuous stamina drain (e.g., sprinting)
	 * @param DrainRate - Stamina drain per second
	 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void StartStaminaDrain(float DrainRate);

	/**
	 * Stop continuous stamina drain
	 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void StopStaminaDrain();

	/**
	 * Restore stamina by amount
	 * @param Amount - Amount to restore (negative values will drain)
	 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void RestoreStamina(float Amount);

	/**
	 * Set stamina to specific value
	 * @param NewStamina - New stamina value (clamped to 0-MaxStamina)
	 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetStamina(float NewStamina);

	/**
	 * Fully restore stamina to maximum
	 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void FullyRestoreStamina();

	/**
	 * Reset regeneration delay timer
	 * @param bFromDepletion - If true, uses depletion delay, otherwise uses usage delay
	 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void ResetRegenDelay(bool bFromDepletion = false);

#pragma endregion Core Functions

#pragma region Queries

	/**
	 * Check if character has enough stamina for an action
	 * @param Required - Required stamina amount
	 * @return True if character has enough stamina
	 */
	UFUNCTION(BlueprintPure, Category = "Stamina")
	bool HasEnoughStamina(float Required) const;

	/**
	 * Check if character can sprint
	 * @return True if character has enough stamina to sprint
	 */
	UFUNCTION(BlueprintPure, Category = "Stamina")
	bool CanSprint() const;

	/**
	 * Check if character can jump
	 * @return True if character has enough stamina to jump
	 */
	UFUNCTION(BlueprintPure, Category = "Stamina")
	bool CanJump() const;

	/**
	 * Get current stamina value
	 * @return Current stamina
	 */
	UFUNCTION(BlueprintPure, Category = "Stamina")
	FORCEINLINE float GetCurrentStamina() const { return CurrentStamina; }

	/**
	 * Get maximum stamina value
	 * @return Maximum stamina
	 */
	UFUNCTION(BlueprintPure, Category = "Stamina")
	FORCEINLINE float GetMaxStamina() const { return MaxStamina; }

	/**
	 * Get stamina as normalized percentage (0.0 - 1.0)
	 * @return Stamina percentage
	 */
	UFUNCTION(BlueprintPure, Category = "Stamina")
	float GetStaminaPercent() const;

	/**
	 * Check if character is exhausted
	 * @return True if exhausted
	 */
	UFUNCTION(BlueprintPure, Category = "Stamina")
	FORCEINLINE bool IsExhausted() const { return bIsExhausted; }

	/**
	 * Check if stamina is regenerating
	 * @return True if regenerating
	 */
	UFUNCTION(BlueprintPure, Category = "Stamina")
	FORCEINLINE bool IsRegenerating() const { return bIsRegenerating; }

	/**
	 * Check if stamina is being drained
	 * @return True if draining
	 */
	UFUNCTION(BlueprintPure, Category = "Stamina")
	FORCEINLINE bool IsDraining() const { return bIsDraining; }

	/**
	 * Get breathing effect intensity (0.0 - 1.0)
	 * Used for visual/audio feedback
	 * @return Breathing intensity
	 */
	UFUNCTION(BlueprintPure, Category = "Stamina|Visual Feedback")
	float GetBreathingIntensity() const;

#pragma endregion Queries

#pragma region Configuration

	/**
	 * Set maximum stamina (also scales current stamina proportionally)
	 * @param NewMax - New maximum stamina value
	 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetMaxStamina(float NewMax);

	/**
	 * Modify stamina regeneration rate
	 * @param NewRate - New regeneration rate per second
	 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetRegenRate(float NewRate);

#pragma endregion Configuration
};