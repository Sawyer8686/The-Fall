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
	Attack		UMETA(DisplayName = "Attack"),
	Custom		UMETA(DisplayName = "Custom")
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TFCHARACTERS_API UTFStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

private:

	FTimerHandle DrainTimerHandle;

#pragma region Stamina Values

	/** Current stamina amount */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true"))
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
	float StaminaRegenRateMoving = 5.0f;

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

public:

	/** Stamina drain per second while sprinting */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Drain", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float SprintDrainRate = 15.0f;

	/** Stamina cost for jumping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Drain", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float JumpStaminaCost = 10.0f;

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina|State", meta = (AllowPrivateAccess = "true"))
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


	/** Handle stamina regeneration logic */
	void RegenerateStamina(float DeltaTime);

	/** Check and update exhaustion state */
	void UpdateExhaustionState();

	/** Calculate current regeneration rate based on character state */
	float GetCurrentRegenRate() const;

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


	UFUNCTION(BlueprintCallable, Category = "Stamina")
	bool ConsumeStamina(float Amount, EStaminaDrainReason Reason = EStaminaDrainReason::Custom);

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void StartStaminaDrain(float DrainRate);

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void StopStaminaDrain();

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void RestoreStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetStamina(float NewStamina);

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void FullyRestoreStamina();

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void ResetRegenDelay(bool bFromDepletion = false);

#pragma endregion Core Functions

#pragma region Queries


	UFUNCTION(BlueprintPure, Category = "Stamina")
	bool HasEnoughStamina(float Required) const;

	UFUNCTION(BlueprintPure, Category = "Stamina")
	bool CanSprint() const;

	UFUNCTION(BlueprintPure, Category = "Stamina")
	bool CanJump() const;

	UFUNCTION(BlueprintPure, Category = "Stamina")
	FORCEINLINE float GetCurrentStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintPure, Category = "Stamina")
	FORCEINLINE float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintPure, Category = "Stamina")
	float GetStaminaPercent() const;

	UFUNCTION(BlueprintPure, Category = "Stamina")
	FORCEINLINE bool IsExhausted() const { return bIsExhausted; }

	UFUNCTION(BlueprintPure, Category = "Stamina")
	FORCEINLINE bool IsRegenerating() const { return bIsRegenerating; }

	UFUNCTION(BlueprintPure, Category = "Stamina")
	FORCEINLINE bool IsDraining() const { return bIsDraining; }

	UFUNCTION(BlueprintPure, Category = "Stamina|Visual Feedback")
	float GetBreathingIntensity() const;

#pragma endregion Queries

#pragma region Configuration


	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetMaxStamina(float NewMax);

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetRegenRate(float NewRate);

#pragma endregion Configuration
};
