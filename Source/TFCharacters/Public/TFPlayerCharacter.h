// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TFCharacterBase.h"
#include "TFKeyHolderInterface.h"
#include "Blueprint/UserWidget.h"
#include "TFPlayerCharacter.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UTFStaminaComponent;
class UTFStatsComponent;
class UTFInteractionComponent;

UENUM()
enum class ESprintBlockReason : uint8
{
	Sneaking,
	NoStamina
};

UCLASS()
class TFCHARACTERS_API ATFPlayerCharacter : public ATFCharacterBase, public ITFKeyHolderInterface
{
	GENERATED_BODY()

private:

#pragma region Components

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UTFStaminaComponent* StaminaComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UTFStatsComponent* StatsComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UTFInteractionComponent* InteractionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* FirstPersonCamera;

#pragma endregion Components

#pragma region Input

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SneakAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LockAction;

#pragma endregion Input

#pragma region Movement & Sprint

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	bool bIsSprinting;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Stamina", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float ExhaustedSpeedMultiplier = 0.7f;

#pragma endregion Movement & Sprint

#pragma region Camera Settings

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FVector FirstPersonCameraOffset = FVector(15.0f, 20.0f, 2.5f);

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FRotator FirstPersonCameraRotation = FRotator(0.0f, -90.0f, 90.0f);

#pragma endregion Camera Settings

protected:

#pragma region Input Handlers

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void SprintOn();
	void SprintOff();
	void SneakOn();
	void SneakOff();
	void PlayerJump();
	void InteractPressed();
	//void InteractReleased();
	void LockPressed();

#pragma endregion Input Handlers

#pragma region Sprint & Stamina

	void SetSprinting(const bool bSprinting);
	float GetSprintSpeed() const;
	virtual void UpdateMovementSpeed() override;

#pragma endregion Sprint & Stamina

#pragma region Stamina Events

	void BindStaminaEvents();
	void UnbindStaminaEvents();
	void HandleStaminaDepleted();
	void HandleStaminaRecovered();
	virtual void OnStaminaDepleted();
	virtual void OnStaminaRecovered();
	virtual void OnSprintBlocked(ESprintBlockReason Reason) {}

#pragma endregion Stamina Events

#pragma region Overrides

	virtual bool CanCharacterJump() const override;
	virtual void HasJumped() override;

#pragma endregion Overrides

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

#pragma region Accessors

	UTFStaminaComponent* GetStaminaComponent() const { return StaminaComponent; }
	UTFStatsComponent* GetStatsComponent() const { return StatsComponent; }
	UTFInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }
	UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }
	bool IsSprinting() const { return bIsSprinting; }

#pragma endregion Accessors

#pragma region Key Collection

protected:

	UPROPERTY(VisibleAnywhere, Category = "Keys")
	TSet<FName> CollectedKeys;

public:

	virtual bool HasKey(FName KeyID) const override;
	virtual void AddKey(FName KeyID) override;
	virtual bool RemoveKey(FName KeyID) override;

	virtual void OnKeyAdded(FName KeyID) {}
	virtual void OnKeyRemoved(FName KeyID) {}

#pragma endregion Key Collection

	ATFPlayerCharacter();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};