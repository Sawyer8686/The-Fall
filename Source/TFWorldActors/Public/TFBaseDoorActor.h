// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TFInteractableActor.h"
#include "TFBaseDoorActor.generated.h"

class UAudioComponent;
class APawn;

UENUM()
enum class EDoorState : uint8
{
	Closed,
	Opening,
	Open,
	Closing
};

UENUM()
enum class EDoorHinge : uint8
{
	Left,
	Right
};

UCLASS()
class TFWORLDACTORS_API ATFBaseDoorActor : public ATFInteractableActor
{
	GENERATED_BODY()

protected:

#pragma region Components

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* DoorMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UAudioComponent* OneShotAudioComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UAudioComponent* LoopAudioComponent;

#pragma endregion Components

#pragma region Door Settings

	UPROPERTY(VisibleAnywhere, Category = "Door|State")
	EDoorState DoorState = EDoorState::Closed;

	UPROPERTY(EditAnywhere, Category = "Door|Settings")
	EDoorHinge HingeType = EDoorHinge::Left;

	UPROPERTY(EditAnywhere, Category = "Door|Settings", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float MaxOpenAngle = 90.0f;

	UPROPERTY(EditAnywhere, Category = "Door|Settings", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float OpenDuration = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Door|Settings", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float CloseDuration = 1.2f;

	UPROPERTY(EditAnywhere, Category = "Door|Settings")
	bool bAutoClose = false;

	UPROPERTY(EditAnywhere, Category = "Door|Settings", meta = (EditCondition = "bAutoClose", ClampMin = "0.0"))
	float AutoCloseDelay = 3.0f;

#pragma endregion Door Settings

#pragma region Animation

	UPROPERTY(VisibleAnywhere, Category = "Door|Animation")
	float CurrentAngle = 0.0f;

	float TargetAngle = 0.0f;
	float AnimationTimer = 0.0f;
	float CurrentAnimationDuration = 0.0f;
	FRotator InitialRotation;
	FTimerHandle AutoCloseTimerHandle;

#pragma endregion Animation

#pragma region Audio

	UPROPERTY(EditAnywhere, Category = "Door|Audio")
	USoundBase* DoorOpenSound;

	UPROPERTY(EditAnywhere, Category = "Door|Audio")
	USoundBase* DoorCloseSound;

	UPROPERTY(EditAnywhere, Category = "Door|Audio")
	USoundBase* DoorMovementSound;

#pragma endregion Audio

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	/** Load door configuration from INI file based on InteractableID */
	virtual void LoadConfigFromINI() override;
	void UpdateDoorAnimation(float DeltaTime);
	void ApplyDoorRotation(float Angle);
	float CalculateTargetAngle(const FVector& PlayerLocation);
	virtual void StartOpening(APawn* OpeningCharacter);
	virtual void StartClosing();
	virtual void CompleteOpening();
	virtual void CompleteClosing();
	void PlayDoorSound(USoundBase* Sound);
	void PlayDoorMovementSound();
	void StopDoorMovementSound();
	void AutoCloseDoor();

public:

	ATFBaseDoorActor();

#pragma region Interface Override

	virtual bool Interact(APawn* InstigatorPawn) override;
	virtual FInteractionData GetInteractionData(APawn* InstigatorPawn) const override;
	virtual bool CanInteract(APawn* InstigatorPawn) const override;

#pragma endregion Interface Override

#pragma region Door Control

	virtual bool OpenDoor(APawn* OpeningCharacter);
	virtual bool CloseDoor();
	virtual bool ToggleDoor(APawn* TogglingCharacter);

#pragma endregion Door Control

#pragma region Events

	virtual void OnDoorStartOpening(APawn* OpeningCharacter) {}
	virtual void OnDoorOpened() {}
	virtual void OnDoorStartClosing() {}
	virtual void OnDoorClosed() {}

#pragma endregion Events

#pragma region Queries

	UStaticMeshComponent* GetDoorFrameMesh() const { return GetMeshComponent(); }
	EDoorState GetDoorState() const { return DoorState; }
	bool IsOpen() const { return DoorState == EDoorState::Open; }
	bool IsClosed() const { return DoorState == EDoorState::Closed; }
	bool IsMoving() const { return DoorState == EDoorState::Opening || DoorState == EDoorState::Closing; }
	float GetDoorOpenPercentage() const;

#pragma endregion Queries
};
