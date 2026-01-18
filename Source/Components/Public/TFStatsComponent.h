// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TFStatsComponent.generated.h"

/** Delegate for stat changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatChanged, float, CurrentValue, float, MaxValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatDepleted, FName, StatName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatCritical, FName, StatName, float, Percent);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COMPONENTS_API UTFStatsComponent : public UActorComponent
{
	GENERATED_BODY()

private:

#pragma region Timer Handles

	FTimerHandle HungerDecayTimerHandle;
	FTimerHandle ThirstDecayTimerHandle;

#pragma endregion Timer Handles

#pragma region Hunger Values

	/** Current hunger amount */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|Hunger", meta = (AllowPrivateAccess = "true"))
	float CurrentHunger;

	/** Maximum hunger capacity */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Hunger", meta = (AllowPrivateAccess = "true", ClampMin = "1.0", ClampMax = "1000.0"))
	float MaxHunger = 100.0f;

	/** Hunger decay amount per tick */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Hunger", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float HungerDecayAmount = 1.0f;

	/** Hunger decay interval in seconds */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Hunger", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float HungerDecayInterval = 5.0f;

	/** Hunger threshold for critical state (0.0 - 1.0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Hunger", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float HungerCriticalThreshold = 0.2f;

#pragma endregion Hunger Values

#pragma region Thirst Values

	/** Current thirst amount */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|Thirst", meta = (AllowPrivateAccess = "true"))
	float CurrentThirst;

	/** Maximum thirst capacity */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Thirst", meta = (AllowPrivateAccess = "true", ClampMin = "1.0", ClampMax = "1000.0"))
	float MaxThirst = 100.0f;

	/** Thirst decay amount per tick */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Thirst", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float ThirstDecayAmount = 1.5f;

	/** Thirst decay interval in seconds */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Thirst", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float ThirstDecayInterval = 4.0f;

	/** Thirst threshold for critical state (0.0 - 1.0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Thirst", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float ThirstCriticalThreshold = 0.2f;

#pragma endregion Thirst Values

#pragma region State

	/** Is hunger at critical level */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|State", meta = (AllowPrivateAccess = "true"))
	bool bIsHungerCritical = false;

	/** Is thirst at critical level */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|State", meta = (AllowPrivateAccess = "true"))
	bool bIsThirstCritical = false;

	/** Is hunger depleted */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|State", meta = (AllowPrivateAccess = "true"))
	bool bIsHungerDepleted = false;

	/** Is thirst depleted */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|State", meta = (AllowPrivateAccess = "true"))
	bool bIsThirstDepleted = false;

#pragma endregion State

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Start decay timers */
	void StartDecayTimers();

	/** Stop decay timers */
	void StopDecayTimers();

	/** Timer callback for hunger decay */
	void DecayHunger();

	/** Timer callback for thirst decay */
	void DecayThirst();

	/** Update hunger critical state */
	void UpdateHungerCriticalState();

	/** Update thirst critical state */
	void UpdateThirstCriticalState();

public:

	UTFStatsComponent();

#pragma region Delegates

	/** Called when hunger value changes */
	UPROPERTY(BlueprintAssignable, Category = "Stats|Events")
	FOnStatChanged OnHungerChanged;

	/** Called when thirst value changes */
	UPROPERTY(BlueprintAssignable, Category = "Stats|Events")
	FOnStatChanged OnThirstChanged;

	/** Called when a stat is completely depleted */
	UPROPERTY(BlueprintAssignable, Category = "Stats|Events")
	FOnStatDepleted OnStatDepleted;

	/** Called when a stat enters critical state */
	UPROPERTY(BlueprintAssignable, Category = "Stats|Events")
	FOnStatCritical OnStatCritical;

#pragma endregion Delegates

#pragma region Hunger Functions

	/** Consume hunger (reduce hunger value) */
	UFUNCTION(BlueprintCallable, Category = "Stats|Hunger")
	void ConsumeHunger(float Amount);

	/** Restore hunger (increase hunger value) */
	UFUNCTION(BlueprintCallable, Category = "Stats|Hunger")
	void RestoreHunger(float Amount);

	/** Set hunger to a specific value */
	UFUNCTION(BlueprintCallable, Category = "Stats|Hunger")
	void SetHunger(float NewHunger);

	/** Fully restore hunger */
	UFUNCTION(BlueprintCallable, Category = "Stats|Hunger")
	void FullyRestoreHunger();

#pragma endregion Hunger Functions

#pragma region Thirst Functions

	/** Consume thirst (reduce thirst value) */
	UFUNCTION(BlueprintCallable, Category = "Stats|Thirst")
	void ConsumeThirst(float Amount);

	/** Restore thirst (increase thirst value) */
	UFUNCTION(BlueprintCallable, Category = "Stats|Thirst")
	void RestoreThirst(float Amount);

	/** Set thirst to a specific value */
	UFUNCTION(BlueprintCallable, Category = "Stats|Thirst")
	void SetThirst(float NewThirst);

	/** Fully restore thirst */
	UFUNCTION(BlueprintCallable, Category = "Stats|Thirst")
	void FullyRestoreThirst();

#pragma endregion Thirst Functions

#pragma region Queries

	UFUNCTION(BlueprintPure, Category = "Stats|Hunger")
	float GetCurrentHunger() const { return CurrentHunger; }

	UFUNCTION(BlueprintPure, Category = "Stats|Hunger")
	float GetMaxHunger() const { return MaxHunger; }

	UFUNCTION(BlueprintPure, Category = "Stats|Hunger")
	float GetHungerPercent() const;

	UFUNCTION(BlueprintPure, Category = "Stats|Hunger")
	bool IsHungerCritical() const { return bIsHungerCritical; }

	UFUNCTION(BlueprintPure, Category = "Stats|Hunger")
	bool IsHungerDepleted() const { return bIsHungerDepleted; }

	UFUNCTION(BlueprintPure, Category = "Stats|Thirst")
	float GetCurrentThirst() const { return CurrentThirst; }

	UFUNCTION(BlueprintPure, Category = "Stats|Thirst")
	float GetMaxThirst() const { return MaxThirst; }

	UFUNCTION(BlueprintPure, Category = "Stats|Thirst")
	float GetThirstPercent() const;

	UFUNCTION(BlueprintPure, Category = "Stats|Thirst")
	bool IsThirstCritical() const { return bIsThirstCritical; }

	UFUNCTION(BlueprintPure, Category = "Stats|Thirst")
	bool IsThirstDepleted() const { return bIsThirstDepleted; }

#pragma endregion Queries

#pragma region Configuration

	UFUNCTION(BlueprintCallable, Category = "Stats|Hunger")
	void SetMaxHunger(float NewMax);

	UFUNCTION(BlueprintCallable, Category = "Stats|Hunger")
	void SetHungerDecayRate(float DecayAmount, float DecayInterval);

	UFUNCTION(BlueprintCallable, Category = "Stats|Thirst")
	void SetMaxThirst(float NewMax);

	UFUNCTION(BlueprintCallable, Category = "Stats|Thirst")
	void SetThirstDecayRate(float DecayAmount, float DecayInterval);

	/** Pause or resume stat decay */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetDecayPaused(bool bPaused);

#pragma endregion Configuration
};
