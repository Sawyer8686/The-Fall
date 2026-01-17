// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TFInteractableActor.h"
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DoorFrameMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DoorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAudioComponent* AudioComponent;

#pragma endregion Components

#pragma region Door Settings

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door|State")
	EDoorState DoorState = EDoorState::Closed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Settings")
	EDoorHinge HingeType = EDoorHinge::Left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Settings", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float MaxOpenAngle = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Settings", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float OpenDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Settings", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float CloseDuration = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Settings")
	bool bAutoClose = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Settings", meta = (EditCondition = "bAutoClose", ClampMin = "0.0"))
	float AutoCloseDelay = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Settings")
	bool bCanOpenFromBothSides = true;

#pragma endregion Door Settings

#pragma region Key Settings

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Key")
	bool bRequiresKey = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Key", meta = (EditCondition = "bRequiresKey"))
	FName RequiredKeyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Key", meta = (EditCondition = "bRequiresKey"))
	FText RequiredKeyName = FText::FromString("Key");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Key", meta = (EditCondition = "bRequiresKey"))
	bool bIsLocked = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Key", meta = (EditCondition = "bRequiresKey"))
	bool bCanRelock = true;

#pragma endregion Key Settings

#pragma region Animation

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door|Animation")
	float CurrentAngle = 0.0f;

	float TargetAngle = 0.0f;
	float AnimationTimer = 0.0f;
	float CurrentAnimationDuration = 0.0f;
	FRotator InitialRotation;
	FTimerHandle AutoCloseTimerHandle;

#pragma endregion Animation

#pragma region Audio

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
	USoundBase* DoorOpenSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
	USoundBase* DoorCloseSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
	USoundBase* DoorLockedSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
	USoundBase* DoorMovementSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio", meta = (EditCondition = "bRequiresKey"))
	USoundBase* DoorUnlockSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio", meta = (EditCondition = "bRequiresKey"))
	USoundBase* DoorLockSound;

#pragma endregion Audio

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void UpdateDoorAnimation(float DeltaTime);
	void ApplyDoorRotation(float Angle);
	float CalculateTargetAngle(const FVector& PlayerLocation);
	virtual void StartOpening(ATFPlayerCharacter* OpeningCharacter);
	virtual void StartClosing();
	virtual void CompleteOpening();
	virtual void CompleteClosing();
	void PlayDoorSound(USoundBase* Sound);
	void AutoCloseDoor();
	bool IsPlayerOnCorrectSide(const FVector& PlayerLocation) const;

public:

	ATFBaseDoorActor();

#pragma region Interface Override

	virtual bool Interact_Implementation(APawn* InstigatorPawn) override;
	virtual FInteractionData GetInteractionData_Implementation(APawn* InstigatorPawn) const override;
	virtual bool CanInteract_Implementation(APawn* InstigatorPawn) const override;

#pragma endregion Interface Override

#pragma region Door Control

	
	UFUNCTION(BlueprintCallable, Category = "Door")
	virtual bool OpenDoor(ATFPlayerCharacter* OpeningCharacter);

	UFUNCTION(BlueprintCallable, Category = "Door")
	virtual bool CloseDoor();

	UFUNCTION(BlueprintCallable, Category = "Door")
	virtual bool ToggleDoor(ATFPlayerCharacter* TogglingCharacter);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Door")
	bool IsDoorLocked() const;
	virtual bool IsDoorLocked_Implementation() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Door")
	bool UnlockDoor(ATFPlayerCharacter* UnlockingCharacter);
	virtual bool UnlockDoor_Implementation(ATFPlayerCharacter* UnlockingCharacter);

	UFUNCTION(BlueprintCallable, Category = "Door|Key")
	virtual bool LockDoor(ATFPlayerCharacter* LockingCharacter);

	UFUNCTION(BlueprintCallable, Category = "Door|Key")
	void SetLockedState(bool bNewLockState);

#pragma endregion Door Control

#pragma region Blueprint Events

	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorStartOpening(ATFPlayerCharacter* OpeningCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorOpened();

	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorStartClosing();

	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorClosed();

	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorLocked(ATFPlayerCharacter* AttemptingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Door|Key")
	void OnDoorUnlocked(ATFPlayerCharacter* UnlockingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Door|Key")
	void OnDoorRelocked(ATFPlayerCharacter* LockingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Door|Key")
	void OnKeyRequired(ATFPlayerCharacter* AttemptingCharacter);

#pragma endregion Blueprint Events

#pragma region Queries

	UFUNCTION(BlueprintPure, Category = "Door")
	FORCEINLINE EDoorState GetDoorState() const { return DoorState; }

	UFUNCTION(BlueprintPure, Category = "Door")
	FORCEINLINE bool IsOpen() const { return DoorState == EDoorState::Open; }

	UFUNCTION(BlueprintPure, Category = "Door")
	FORCEINLINE bool IsClosed() const { return DoorState == EDoorState::Closed; }

	UFUNCTION(BlueprintPure, Category = "Door")
	FORCEINLINE bool IsMoving() const { return DoorState == EDoorState::Opening || DoorState == EDoorState::Closing; }

	UFUNCTION(BlueprintPure, Category = "Door")
	float GetDoorOpenPercentage() const;

	UFUNCTION(BlueprintPure, Category = "Door|Key")
	FORCEINLINE FName GetRequiredKeyID() const { return RequiredKeyID; }

	UFUNCTION(BlueprintPure, Category = "Door|Key")
	FORCEINLINE FText GetRequiredKeyName() const { return RequiredKeyName; }

	UFUNCTION(BlueprintPure, Category = "Door|Key")
	bool CharacterHasKey(const ATFPlayerCharacter* Character) const;

	UFUNCTION(BlueprintPure, Category = "Door|Key")
	FORCEINLINE bool IsLocked() const { return bRequiresKey && bIsLocked; }

	UFUNCTION(BlueprintPure, Category = "Door|Key")
	FORCEINLINE bool RequiresKey() const { return bRequiresKey; }

#pragma endregion Queries
};