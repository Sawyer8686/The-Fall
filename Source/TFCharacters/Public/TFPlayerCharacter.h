// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TFCharacterBase.h"
#include "Blueprint/UserWidget.h"
#include "TFPlayerCharacter.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UTFStaminaComponent;
class UTFInteractionComponent;

/**
 * Player Character class with camera system, stamina management, and interaction
 * Supports both First Person and Third Person views with smooth transitions
 * Includes sprint system with stamina consumption
 */
UCLASS(Blueprintable)
class TFCHARACTERS_API ATFPlayerCharacter : public ATFCharacterBase
{
	GENERATED_BODY()

private:

#pragma region Components

	/** Stamina management component (player only) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UTFStaminaComponent* StaminaComponent;

	/** Interaction component for world object interaction */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UTFInteractionComponent* InteractionComponent;

	/** First person camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCamera;

#pragma endregion Components

#pragma region Input 

	/** Default input mapping context */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Move input action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look input action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Jump input action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Sprint input action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	/** Sneak input action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* SneakAction;

	/** Interact input action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

#pragma endregion Input

#pragma region Movement & Sprint

	/** Is character currently sprinting */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsSprinting;

	/** Sprint movement speed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float SprintSpeed = 300.f;

	/** Speed multiplier when exhausted from stamina depletion */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Stamina", meta = (AllowPrivateAccess = "true", ClampMin = "0.1", ClampMax = "1.0"))
	float ExhaustedSpeedMultiplier = 0.7f;

#pragma endregion Movement & Sprint

#pragma region Camera Settings

	/** First person camera offset from head socket */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	FVector FirstPersonCameraOffset = FVector(15.0f, 20.0f, 2.5f);

	/** First person camera rotation relative to head socket */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	FRotator FirstPersonCameraRotation = FRotator(0.0f, -90.0f, 90.0f);

#pragma endregion Camera Settings

protected:

#pragma region Input Handlers

	/** Handle movement input */
	void Move(const FInputActionValue& Value);

	/** Handle look input */
	void Look(const FInputActionValue& Value);

	/** Start sprinting */
	void SprintOn();

	/** Stop sprinting */
	void SprintOff();

	/** Start sneaking */
	void SneakOn();

	/** Stop sneaking */
	void SneakOff();

	/** Handle jump input */
	void PlayerJump();

	/** Handle interact input (press) */
	void InteractPressed();

	/** Handle interact input (release) */
	void InteractReleased();

#pragma endregion Input Handlers

#pragma region Sprint & Stamina

	/** Set sprinting state (stamina aware) */
	void SetSprinting(const bool bSprinting);

	/** Get current sprint speed */
	float GetSprintSpeed() const;

	/** Update movement speed based on sprint/sneak/exhaustion state */
	virtual void UpdateMovementSpeed() override;

#pragma endregion Sprint & Stamina

#pragma region Stamina Events

	/** Bind to stamina component events */
	void BindStaminaEvents();

	/** Handle stamina depletion */
	UFUNCTION()
	void HandleStaminaDepleted();

	/** Handle stamina recovery */
	UFUNCTION()
	void HandleStaminaRecovered();

	/** Called when stamina is depleted (Blueprint implementable) */
	UFUNCTION(BlueprintNativeEvent, Category = "Stamina")
	void OnStaminaDepleted();
	virtual void OnStaminaDepleted_Implementation();

	/** Called when stamina recovers from exhaustion (Blueprint implementable) */
	UFUNCTION(BlueprintNativeEvent, Category = "Stamina")
	void OnStaminaRecovered();
	virtual void OnStaminaRecovered_Implementation();

#pragma endregion Stamina Events

#pragma region Overrides

	/** Override: Check if character can jump (stamina aware) */
	virtual bool CanCharacterJump() const override;

	/** Override: Execute jump (consumes stamina) */
	virtual void HasJumped() override;

#pragma endregion Overrides

	virtual void BeginPlay() override;

public:

#pragma region Accessors

	/** Get stamina component */
	UFUNCTION(BlueprintPure, Category = "Components")
	FORCEINLINE UTFStaminaComponent* GetStaminaComponent() const { return StaminaComponent; }

	/** Get interaction component */
	UFUNCTION(BlueprintPure, Category = "Components")
	FORCEINLINE UTFInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

	/** Get first person camera component */
	FORCEINLINE UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

	/** Check if currently sprinting */
	UFUNCTION(BlueprintPure, Category = "Movement")
	FORCEINLINE bool IsSprinting() const { return bIsSprinting; }

#pragma endregion Accessors

#pragma region Key Collection

	protected:

		/** Collection of keys the player has acquired */
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Keys", meta = (AllowPrivateAccess = "true"))
		TSet<FName> CollectedKeys;

  public:

	  /**
	   * Check if player has a specific key
	   * @param KeyID - The key identifier to check for
	   * @return True if player has the key
	   */
	  UFUNCTION(BlueprintPure, Category = "Keys")
	  bool HasKey(FName KeyID) const;

	  /**
	   * Add a key to player's collection
	   * @param KeyID - The key identifier to add
	   */
	  UFUNCTION(BlueprintCallable, Category = "Keys")
	  void AddKey(FName KeyID);

	  /**
	   * Remove a key from player's collection
	   * @param KeyID - The key identifier to remove
	   * @return True if key was removed
	   */
	  UFUNCTION(BlueprintCallable, Category = "Keys")
	  bool RemoveKey(FName KeyID);

	  /** Called when a key is added to collection */
	  UFUNCTION(BlueprintImplementableEvent, Category = "Keys")
	  void OnKeyAdded(FName KeyID);

	  /** Called when a key is removed from collection */
	  UFUNCTION(BlueprintImplementableEvent, Category = "Keys")
	  void OnKeyRemoved(FName KeyID);

#pragma endregion Key Collection

	ATFPlayerCharacter();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};