// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TFCharacterBase.generated.h"

class UTFStaminaComponent;

UCLASS(Abstract, NotBlueprintable)
class TFCHARACTERS_API ATFCharacterBase : public ACharacter
{
	GENERATED_BODY()

private:

#pragma region Components

	/** Stamina management component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UTFStaminaComponent* StaminaComponent;

#pragma endregion Components

#pragma region Movement

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsSprinting;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsSneaking;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float SneakSpeed = 35.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float WalkSpeed = 150.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float SprintSpeed = 300.f;

	/** Speed multiplier when exhausted */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.1", ClampMax = "1.0"))
	float ExhaustedSpeedMultiplier = 0.7f;

#pragma endregion Movement

protected:

	virtual void BeginPlay() override;

	/** Check if character can jump (stamina aware) */
	bool CanCharacterJump() const;

	/** Execute jump (consumes stamina) */
	void HasJumped();

	float GetSneakSpeed() const;
	float GetWalkSpeed() const;
	float GetSprintSpeed() const;

	/** Set sprinting state (stamina aware) */
	void SetSprinting(const bool bSprinting);

	/** Set sneaking state */
	void SetSneaking(const bool bSneaking);

	/** Called when stamina is depleted */
	UFUNCTION(BlueprintImplementableEvent, Category = "Stamina")
	void OnStaminaDepleted();

	/** Called when stamina recovers from exhaustion */
	UFUNCTION(BlueprintImplementableEvent, Category = "Stamina")
	void OnStaminaRecovered();

	/** Update movement speed based on exhaustion state */
	void UpdateMovementSpeed();

private:

	/** Bind to stamina component events */
	void BindStaminaEvents();

	/** Handle stamina depletion */
	UFUNCTION()
	void HandleStaminaDepleted();

	/** Handle stamina recovery */
	UFUNCTION()
	void HandleStaminaRecovered();

public:

	ATFCharacterBase();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Get stamina component */
	UFUNCTION(BlueprintPure, Category = "Components")
	FORCEINLINE UTFStaminaComponent* GetStaminaComponent() const { return StaminaComponent; }
};