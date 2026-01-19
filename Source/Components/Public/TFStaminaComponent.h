// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TFStaminaComponent.generated.h"

class ACharacter;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float, float);
DECLARE_MULTICAST_DELEGATE(FOnStaminaDepleted);
DECLARE_MULTICAST_DELEGATE(FOnStaminaRecovered);

UENUM()
enum class EStaminaDrainReason : uint8
{
	Sprint,
	Jump,
	Custom
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COMPONENTS_API UTFStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

private:

	FTimerHandle DrainTimerHandle;

#pragma region Stamina Values

	UPROPERTY(VisibleAnywhere, Category = "Stamina")
	float CurrentStamina;

	UPROPERTY(EditDefaultsOnly, Category = "Stamina", meta = (ClampMin = "1.0", ClampMax = "1000.0"))
	float MaxStamina = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stamina|Exhaustion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ExhaustionThreshold = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stamina|Exhaustion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ExhaustionRecoveryThreshold = 0.50f;

#pragma endregion Stamina Values

#pragma region Regeneration

	UPROPERTY(EditDefaultsOnly, Category = "Stamina|Regeneration", meta = (ClampMin = "0.0"))
	float StaminaRegenRate = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stamina|Regeneration", meta = (ClampMin = "0.0"))
	float StaminaRegenRateMoving = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stamina|Regeneration", meta = (ClampMin = "0.0"))
	float RegenDelayAfterDepletion = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stamina|Regeneration", meta = (ClampMin = "0.0"))
	float RegenDelayAfterUsage = 2.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Stamina|Exhaustion", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float ExhaustedRegenMultiplier = 0.5f;

#pragma endregion Regeneration

#pragma region Drain Costs

public:

	UPROPERTY(EditDefaultsOnly, Category = "Stamina|Drain", meta = (ClampMin = "0.0"))
	float SprintDrainRate = 15.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stamina|Drain", meta = (ClampMin = "0.0"))
	float JumpStaminaCost = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stamina|Drain", meta = (ClampMin = "0.0"))
	float MinStaminaToSprint = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stamina|Drain", meta = (ClampMin = "0.0"))
	float MinStaminaToJump = 5.0f;

#pragma endregion Drain Costs

#pragma region State

	UPROPERTY(VisibleAnywhere, Category = "Stamina|State")
	bool bIsRegenerating = true;

	UPROPERTY(VisibleAnywhere, Category = "Stamina|State")
	bool bIsExhausted = false;

	UPROPERTY(VisibleAnywhere, Category = "Stamina|State")
	bool bIsDraining = false;

	float RegenDelayTimer = 0.0f;

	UPROPERTY()
	ACharacter* OwnerCharacter;

#pragma endregion State

#pragma region Visual Feedback

	UPROPERTY(EditDefaultsOnly, Category = "Stamina|Visual Feedback")
	bool bEnableBreathingEffects = true;

	UPROPERTY(EditDefaultsOnly, Category = "Stamina|Visual Feedback", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BreathingEffectThreshold = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "Stamina|Visual Feedback", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float BreathingIntensity = 1.0f;

#pragma endregion Visual Feedback

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
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

	FOnStaminaChanged OnStaminaChanged;
	FOnStaminaDepleted OnStaminaDepleted;
	FOnStaminaRecovered OnStaminaRecovered;

#pragma endregion Delegates

#pragma region Core Functions

	bool ConsumeStamina(float Amount, EStaminaDrainReason Reason = EStaminaDrainReason::Custom);
	void StartStaminaDrain(float DrainRate);
	void StopStaminaDrain();
	void RestoreStamina(float Amount);
	void SetStamina(float NewStamina);
	void FullyRestoreStamina();
	void ResetRegenDelay(bool bFromDepletion = false);

#pragma endregion Core Functions

#pragma region Queries

	bool HasEnoughStamina(float Required) const;
	bool CanSprint() const;
	bool CanJump() const;
	float GetCurrentStamina() const { return CurrentStamina; }
	float GetMaxStamina() const { return MaxStamina; }
	float GetStaminaPercent() const;
	bool IsExhausted() const { return bIsExhausted; }
	bool IsRegenerating() const { return bIsRegenerating; }
	bool IsDraining() const { return bIsDraining; }
	float GetBreathingIntensity() const;

#pragma endregion Queries

#pragma region Configuration

	void SetMaxStamina(float NewMax);
	void SetRegenRate(float NewRate);

#pragma endregion Configuration

#pragma region Modifiers

	UPROPERTY(VisibleAnywhere, Category = "Stamina|Modifiers")
	float DrainRateMultiplier = 1.0f;

	UPROPERTY(VisibleAnywhere, Category = "Stamina|Modifiers")
	float RegenRateMultiplier = 1.0f;

	void SetDrainRateMultiplier(float Multiplier);
	void SetRegenRateMultiplier(float Multiplier);
	void ResetModifiers();
	float GetEffectiveDrainRate(float BaseDrainRate) const;
	float GetEffectiveRegenRate() const;

#pragma endregion Modifiers

	void EnsureTickEnabledIfNeeded();
};
