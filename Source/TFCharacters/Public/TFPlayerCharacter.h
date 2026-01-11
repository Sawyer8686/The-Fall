// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TFCharacterBase.h"
#include "TFPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * Player Character class with smooth camera perspective transition
 * Supports both First Person and Third Person views with configurable transitions
 */
UCLASS(Blueprintable)
class TFCHARACTERS_API ATFPlayerCharacter : public ATFCharacterBase
{
	GENERATED_BODY()

private:

#pragma region Camera Components

	/** Spring arm for third person camera positioning */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Third person camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* ThirdPersonCamera;

	/** First person camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCamera;

#pragma endregion Camera Components

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

	/** Toggle camera perspective input action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleCameraPerspective;

	/** Sprint input action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	/** Sneak input action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* SneakAction;

#pragma endregion Input

#pragma region Camera Settings

	/** Current perspective state */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	bool bInFirstPerson = true;

	/** First person camera offset from head socket */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|First Person", meta = (AllowPrivateAccess = "true"))
	FVector FirstPersonCameraOffset = FVector(15.0f, 20.0f, 2.5f);

	/** First person camera rotation relative to head socket */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|First Person", meta = (AllowPrivateAccess = "true"))
	FRotator FirstPersonCameraRotation = FRotator(0.0f, -90.0f, 90.0f);

	/** Distance of third person camera from character */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Third Person", meta = (AllowPrivateAccess = "true"))
	float ThirdPersonCameraDistance = 400.0f;

#pragma endregion Camera Settings

#pragma region Rotation Settings

	/** Character rotation rate in third person mode */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Rotation", meta = (AllowPrivateAccess = "true"))
	FRotator ThirdPersonRotationRate = FRotator(0.0f, 500.0f, 0.0f);

	/** Character rotation rate in first person mode */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Rotation", meta = (AllowPrivateAccess = "true"))
	FRotator FirstPersonRotationRate = FRotator(0.0f, 720.0f, 0.0f);

#pragma endregion Rotation Settings

#pragma region Transition Settings

	/** Duration of camera transition animation (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Transition", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "2.0"))
	float CameraTransitionDuration = 0.3f;

	/** Duration of rotation transition animation (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Transition", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "2.0"))
	float RotationTransitionDuration = 0.25f;

	/** Enable smooth camera transition between perspectives */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Transition", meta = (AllowPrivateAccess = "true"))
	bool bSmoothCameraTransition = true;

	/** Enable smooth character rotation during transition */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Transition", meta = (AllowPrivateAccess = "true"))
	bool bSmoothRotationTransition = true;

#pragma endregion Transition Settings

#pragma region Transition State

	/** Is camera currently transitioning between perspectives */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera|Transition", meta = (AllowPrivateAccess = "true"))
	bool bIsTransitioning = false;

	/** Current transition timer */
	float TransitionTimer = 0.0f;

	/** Current transition alpha (0.0 to 1.0) */
	float TransitionAlpha = 0.0f;

	/** Camera arm length at start of transition */
	float StartCameraArmLength = 0.0f;

	/** Target camera arm length for transition */
	float TargetCameraArmLength = 0.0f;

	/** Controller rotation at start of transition */
	FRotator StartControlRotation;

	/** Target controller rotation for transition */
	FRotator TargetControlRotation;

	/** Character rotation at start of transition */
	FRotator StartCharacterRotation;

#pragma endregion Transition State

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

	/** Toggle between first and third person perspective */
	void TogglePerspective();

#pragma endregion Input Handlers

#pragma region Camera Configuration

	/** Configure character for first person mode */
	void ConfigureFirstPersonMode();

	/** Configure character for third person mode */
	void ConfigureThirdPersonMode();

#pragma endregion Camera Configuration

#pragma region Transition System

	/**
	 * Start smooth transition between camera perspectives
	 * @param bToFirstPerson - True if transitioning to first person, false for third person
	 */
	void StartCameraTransition(bool bToFirstPerson);

	/**
	 * Update camera transition interpolation
	 * @param DeltaTime - Time since last frame
	 */
	void UpdateCameraTransition(float DeltaTime);

	/** Complete camera transition and finalize configuration */
	void CompleteCameraTransition();

#pragma endregion Transition System

	virtual void BeginPlay() override;

public:

#pragma region Accessors

	/** Get camera boom component */
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Get third person camera component */
	FORCEINLINE UCameraComponent* GetThirdPersonCamera() const { return ThirdPersonCamera; }

	/** Get first person camera component */
	FORCEINLINE UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

	/** Check if character is in first person mode */
	FORCEINLINE bool IsInFirstPerson() const { return bInFirstPerson; }

	/** Check if camera is currently transitioning */
	FORCEINLINE bool IsTransitioning() const { return bIsTransitioning; }

#pragma endregion Accessors

	ATFPlayerCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};