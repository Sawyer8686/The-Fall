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
class UTFInteractionComponent;

UCLASS(Blueprintable)
class TFCHARACTERS_API ATFPlayerCharacter : public ATFCharacterBase, public ITFKeyHolderInterface
{
	GENERATED_BODY()

private:

#pragma region Components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UTFStaminaComponent* StaminaComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UTFInteractionComponent* InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCamera;

#pragma endregion Components

#pragma region Input 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* SneakAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* LockAction;

#pragma endregion Input

#pragma region Movement & Sprint

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsSprinting;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float SprintSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Stamina", meta = (AllowPrivateAccess = "true", ClampMin = "0.1", ClampMax = "1.0"))
	float ExhaustedSpeedMultiplier = 0.7f;

#pragma endregion Movement & Sprint

#pragma region Camera Settings

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	FVector FirstPersonCameraOffset = FVector(15.0f, 20.0f, 2.5f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
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

	UFUNCTION()
	void HandleStaminaDepleted();

	UFUNCTION()
	void HandleStaminaRecovered();

	UFUNCTION(BlueprintNativeEvent, Category = "Stamina")
	void OnStaminaDepleted();
	virtual void OnStaminaDepleted_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Stamina")
	void OnStaminaRecovered();
	virtual void OnStaminaRecovered_Implementation();

#pragma endregion Stamina Events

#pragma region Overrides

	virtual bool CanCharacterJump() const override;
	virtual void HasJumped() override;

#pragma endregion Overrides

	virtual void BeginPlay() override;

public:

#pragma region Accessors

	UFUNCTION(BlueprintPure, Category = "Components")
	FORCEINLINE UTFStaminaComponent* GetStaminaComponent() const { return StaminaComponent; }

	UFUNCTION(BlueprintPure, Category = "Components")
	FORCEINLINE UTFInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

	FORCEINLINE UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

	UFUNCTION(BlueprintPure, Category = "Movement")
	FORCEINLINE bool IsSprinting() const { return bIsSprinting; }

#pragma endregion Accessors

#pragma region Key Collection

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Keys", meta = (AllowPrivateAccess = "true"))
	TSet<FName> CollectedKeys;

public:

	
	virtual bool HasKey_Implementation(FName KeyID) const override;

	virtual void AddKey_Implementation(FName KeyID) override;

	virtual bool RemoveKey_Implementation(FName KeyID) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Keys")
	void OnKeyAdded(FName KeyID);

	UFUNCTION(BlueprintImplementableEvent, Category = "Keys")
	void OnKeyRemoved(FName KeyID);

#pragma endregion Key Collection

	ATFPlayerCharacter();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};