// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TFStatsComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStatChanged, float, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnStatDepleted, FName);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStatCritical, FName, float);

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

	UPROPERTY(VisibleAnywhere, Category = "Stats|Hunger")
	float CurrentHunger;

	UPROPERTY(EditDefaultsOnly, Category = "Stats|Hunger", meta = (ClampMin = "1.0", ClampMax = "1000.0"))
	float MaxHunger = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats|Hunger", meta = (ClampMin = "0.0"))
	float HungerDecayAmount = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats|Hunger", meta = (ClampMin = "0.1"))
	float HungerDecayInterval = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats|Hunger", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HungerCriticalThreshold = 0.2f;

#pragma endregion Hunger Values

#pragma region Thirst Values

	UPROPERTY(VisibleAnywhere, Category = "Stats|Thirst")
	float CurrentThirst;

	UPROPERTY(EditDefaultsOnly, Category = "Stats|Thirst", meta = (ClampMin = "1.0", ClampMax = "1000.0"))
	float MaxThirst = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats|Thirst", meta = (ClampMin = "0.0"))
	float ThirstDecayAmount = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats|Thirst", meta = (ClampMin = "0.1"))
	float ThirstDecayInterval = 4.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats|Thirst", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ThirstCriticalThreshold = 0.2f;

#pragma endregion Thirst Values

#pragma region State

	UPROPERTY(VisibleAnywhere, Category = "Stats|State")
	bool bIsHungerCritical = false;

	UPROPERTY(VisibleAnywhere, Category = "Stats|State")
	bool bIsThirstCritical = false;

	UPROPERTY(VisibleAnywhere, Category = "Stats|State")
	bool bIsHungerDepleted = false;

	UPROPERTY(VisibleAnywhere, Category = "Stats|State")
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

	FOnStatChanged OnHungerChanged;
	FOnStatChanged OnThirstChanged;
	FOnStatDepleted OnStatDepleted;
	FOnStatCritical OnStatCritical;

#pragma endregion Delegates

#pragma region Hunger Functions

	void ConsumeHunger(float Amount);
	void RestoreHunger(float Amount);
	void SetHunger(float NewHunger);
	void FullyRestoreHunger();

#pragma endregion Hunger Functions

#pragma region Thirst Functions

	void ConsumeThirst(float Amount);
	void RestoreThirst(float Amount);
	void SetThirst(float NewThirst);
	void FullyRestoreThirst();

#pragma endregion Thirst Functions

#pragma region Queries

	float GetCurrentHunger() const { return CurrentHunger; }
	float GetMaxHunger() const { return MaxHunger; }
	float GetHungerPercent() const;
	bool IsHungerCritical() const { return bIsHungerCritical; }
	bool IsHungerDepleted() const { return bIsHungerDepleted; }
	float GetCurrentThirst() const { return CurrentThirst; }
	float GetMaxThirst() const { return MaxThirst; }
	float GetThirstPercent() const;
	bool IsThirstCritical() const { return bIsThirstCritical; }
	bool IsThirstDepleted() const { return bIsThirstDepleted; }

#pragma endregion Queries

#pragma region Configuration

	void SetMaxHunger(float NewMax);
	void SetHungerDecayRate(float DecayAmount, float DecayInterval);
	void SetMaxThirst(float NewMax);
	void SetThirstDecayRate(float DecayAmount, float DecayInterval);
	void SetDecayPaused(bool bPaused);

#pragma endregion Configuration
};
