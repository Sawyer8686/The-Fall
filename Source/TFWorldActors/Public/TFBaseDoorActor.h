// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/TFInteractableActor.h"
#include "TFBaseDoorActor.generated.h"

class UAudioComponent;

UENUM(BlueprintType)
enum class EDoorState : uint8
{
	Closed		UMETA(DisplayName = "Closed"),
	Opening		UMETA(DisplayName = "Opening"),
	Open		UMETA(DisplayName = "Open"),
	Closing		UMETA(DisplayName = "Closing")
};

/** Door hinge side */
UENUM(BlueprintType)
enum class EDoorHinge : uint8
{
	Left		UMETA(DisplayName = "Left"),
	Right		UMETA(DisplayName = "Right")
};

UCLASS(Blueprintable)
class TFWORLDACTORS_API ATFBaseDoorActor : public ATFInteractableActor
{
	GENERATED_BODY()

protected:

#pragma region Components

	/** Door frame mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DoorFrameMesh;

	/** Door mesh (the moving part) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DoorMesh;

	/** Audio component for door sounds */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAudioComponent* AudioComponent;

#pragma endregion Components

#pragma region Door Settings

	/** Current state of the door */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door|State")
	EDoorState DoorState = EDoorState::Closed;

	/** Which side the hinge is on */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Settings")
	EDoorHinge HingeType = EDoorHinge::Left;

	/** Maximum angle the door can open (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Settings", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float MaxOpenAngle = 90.0f;

	/** Time it takes to open the door (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Settings", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float OpenDuration = 1.0f;

	/** Time it takes to close the door (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Settings", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float CloseDuration = 1.2f;

	/** Should door auto-close after being opened */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Settings")
	bool bAutoClose = false;

	/** Delay before auto-closing (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Settings", meta = (EditCondition = "bAutoClose", ClampMin = "0.0"))
	float AutoCloseDelay = 3.0f;

	/** Can door be opened from both sides */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Settings")
	bool bCanOpenFromBothSides = true;

#pragma endregion Door Settings

#pragma region Key Settings

	/** Does this door require a key to unlock */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Key")
	bool bRequiresKey = false;

	/** Key ID required to unlock/lock this door */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Key", meta = (EditCondition = "bRequiresKey"))
	FName RequiredKeyID = NAME_None;

	/** Display name of the required key (for UI) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Key", meta = (EditCondition = "bRequiresKey"))
	FText RequiredKeyName = FText::FromString("Key");

	/** Is the door currently in a locked state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Key", meta = (EditCondition = "bRequiresKey"))
	bool bIsLocked = true;

	/** Can this door be re-locked after unlocking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Key", meta = (EditCondition = "bRequiresKey"))
	bool bCanRelock = true;

#pragma endregion Key Settings

#pragma region Animation

	/** Current door rotation angle */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door|Animation")
	float CurrentAngle = 0.0f;

	/** Target angle for animation */
	float TargetAngle = 0.0f;

	/** Animation timer */
	float AnimationTimer = 0.0f;

	/** Current animation duration */
	float CurrentAnimationDuration = 0.0f;

	/** Initial door rotation */
	FRotator InitialRotation;

	/** Timer handle for auto-close */
	FTimerHandle AutoCloseTimerHandle;

#pragma endregion Animation

#pragma region Audio

	/** Sound played when door starts opening */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
	USoundBase* DoorOpenSound;

	/** Sound played when door starts closing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
	USoundBase* DoorCloseSound;

	/** Sound played when door is locked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
	USoundBase* DoorLockedSound;

	/** Sound played during door movement (looping) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
	USoundBase* DoorMovementSound;

	/** Sound played when door is unlocked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio", meta = (EditCondition = "bRequiresKey"))
	USoundBase* DoorUnlockSound;

	/** Sound played when door is locked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio", meta = (EditCondition = "bRequiresKey"))
	USoundBase* DoorLockSound;

#pragma endregion Audio

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Update door animation */
	void UpdateDoorAnimation(float DeltaTime);

	/** Apply rotation to door mesh */
	void ApplyDoorRotation(float Angle);

	/** Calculate target angle based on player position */
	float CalculateTargetAngle(const FVector& PlayerLocation);

	/** Start door opening animation */
	virtual void StartOpening(ATFPlayerCharacter* OpeningCharacter);

	/** Start door closing animation */
	virtual void StartClosing();

	/** Complete door opening */
	virtual void CompleteOpening();

	/** Complete door closing */
	virtual void CompleteClosing();

	/** Play door sound */
	void PlayDoorSound(USoundBase* Sound);

	/** Auto-close callback */
	void AutoCloseDoor();

	/** Check if player is on the correct side to open door */
	bool IsPlayerOnCorrectSide(const FVector& PlayerLocation) const;

public:

	ATFBaseDoorActor();

#pragma region Interface Override

	/** Override interact to handle door open/close */
	virtual bool Interact_Implementation(ATFPlayerCharacter* InstigatorCharacter) override;

	/** Override interaction data to show door state */
	virtual FInteractionData GetInteractionData_Implementation(ATFPlayerCharacter* InstigatorCharacter) const override;

	/** Override can interact to check if door can be used */
	virtual bool CanInteract_Implementation(ATFPlayerCharacter* InstigatorCharacter) const override;

#pragma endregion Interface Override

#pragma region Door Control

	/**
	 * Open the door
	 * @param OpeningCharacter - Character opening the door
	 * @return True if door started opening
	 */
	UFUNCTION(BlueprintCallable, Category = "Door")
	virtual bool OpenDoor(ATFPlayerCharacter* OpeningCharacter);

	/**
	 * Close the door
	 * @return True if door started closing
	 */
	UFUNCTION(BlueprintCallable, Category = "Door")
	virtual bool CloseDoor();

	/**
	 * Toggle door open/close
	 * @param TogglingCharacter - Character toggling the door
	 * @return True if action succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Door")
	virtual bool ToggleDoor(ATFPlayerCharacter* TogglingCharacter);

	/**
	 * Check if door is locked
	 * @return True if door requires key and is in locked state
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Door")
	bool IsDoorLocked() const;
	virtual bool IsDoorLocked_Implementation() const;

	/**
	 * Unlock the door (requires player to have the key if bRequiresKey)
	 * @param UnlockingCharacter - Character unlocking the door
	 * @return True if door was unlocked
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Door")
	bool UnlockDoor(ATFPlayerCharacter* UnlockingCharacter);
	virtual bool UnlockDoor_Implementation(ATFPlayerCharacter* UnlockingCharacter);

	/**
	 * Lock the door (requires player to have the key)
	 * @param LockingCharacter - Character attempting to lock
	 * @return True if door was locked
	 */
	UFUNCTION(BlueprintCallable, Category = "Door|Key")
	virtual bool LockDoor(ATFPlayerCharacter* LockingCharacter);

	/**
	 * Force set lock state (ignores key requirement - for puzzles/triggers)
	 * @param bNewLockState - New lock state
	 */
	UFUNCTION(BlueprintCallable, Category = "Door|Key")
	void SetLockedState(bool bNewLockState);

#pragma endregion Door Control

#pragma region Blueprint Events

	/** Called when door starts opening */
	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorStartOpening(ATFPlayerCharacter* OpeningCharacter);

	/** Called when door finishes opening */
	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorOpened();

	/** Called when door starts closing */
	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorStartClosing();

	/** Called when door finishes closing */
	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorClosed();

	/** Called when player tries to open locked door */
	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorLocked(ATFPlayerCharacter* AttemptingCharacter);

	/** Called when door is unlocked with a key */
	UFUNCTION(BlueprintImplementableEvent, Category = "Door|Key")
	void OnDoorUnlocked(ATFPlayerCharacter* UnlockingCharacter);

	/** Called when door is locked with a key */
	UFUNCTION(BlueprintImplementableEvent, Category = "Door|Key")
	void OnDoorRelocked(ATFPlayerCharacter* LockingCharacter);

	/** Called when player tries to open without key */
	UFUNCTION(BlueprintImplementableEvent, Category = "Door|Key")
	void OnKeyRequired(ATFPlayerCharacter* AttemptingCharacter);

#pragma endregion Blueprint Events

#pragma region Queries

	/** Get current door state */
	UFUNCTION(BlueprintPure, Category = "Door")
	FORCEINLINE EDoorState GetDoorState() const { return DoorState; }

	/** Check if door is open */
	UFUNCTION(BlueprintPure, Category = "Door")
	FORCEINLINE bool IsOpen() const { return DoorState == EDoorState::Open; }

	/** Check if door is closed */
	UFUNCTION(BlueprintPure, Category = "Door")
	FORCEINLINE bool IsClosed() const { return DoorState == EDoorState::Closed; }

	/** Check if door is moving */
	UFUNCTION(BlueprintPure, Category = "Door")
	FORCEINLINE bool IsMoving() const { return DoorState == EDoorState::Opening || DoorState == EDoorState::Closing; }

	/** Get door open percentage (0.0 - 1.0) */
	UFUNCTION(BlueprintPure, Category = "Door")
	float GetDoorOpenPercentage() const;

	/** Get the required key ID */
	UFUNCTION(BlueprintPure, Category = "Door|Key")
	FORCEINLINE FName GetRequiredKeyID() const { return RequiredKeyID; }

	/** Get the required key display name */
	UFUNCTION(BlueprintPure, Category = "Door|Key")
	FORCEINLINE FText GetRequiredKeyName() const { return RequiredKeyName; }

	/** Check if a character has the required key */
	UFUNCTION(BlueprintPure, Category = "Door|Key")
	bool CharacterHasKey(const ATFPlayerCharacter* Character) const;

	/** Check if door is in locked state */
	UFUNCTION(BlueprintPure, Category = "Door|Key")
	FORCEINLINE bool IsLocked() const { return bRequiresKey && bIsLocked; }

	/** Check if this door requires a key */
	UFUNCTION(BlueprintPure, Category = "Door|Key")
	FORCEINLINE bool RequiresKey() const { return bRequiresKey; }

#pragma endregion Queries
};