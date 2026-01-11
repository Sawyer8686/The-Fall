// Fill out your copyright notice in the Description page of Project Settings.

#include "TFPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"

ATFPlayerCharacter::ATFPlayerCharacter()
{
	// Enable tick for smooth camera transitions
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = false;

	// Capsule component setup
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Character movement configuration
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Camera boom setup (spring arm for third person)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = ThirdPersonCameraDistance;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.0f;

	// Third person camera setup
	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	ThirdPersonCamera->bUsePawnControlRotation = false;
	ThirdPersonCamera->Deactivate();
	ThirdPersonCamera->bAutoActivate = false;

	// First person camera setup (attached to head socket)
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), "head");
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->Activate();
	FirstPersonCamera->SetRelativeLocation(FirstPersonCameraOffset);
	FirstPersonCamera->SetRelativeRotation(FirstPersonCameraRotation);

	// Initialize in first person mode
	ConfigureFirstPersonMode();
}

void ATFPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Apply correct camera mode on begin play
	if (bInFirstPerson)
	{
		CameraBoom->TargetArmLength = 0.0f;
		ConfigureFirstPersonMode();
	}
	else
	{
		CameraBoom->TargetArmLength = ThirdPersonCameraDistance;
		ConfigureThirdPersonMode();
	}
}

void ATFPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update camera transition if active
	if (bIsTransitioning)
	{
		UpdateCameraTransition(DeltaTime);
	}
}

void ATFPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Setup enhanced input mapping context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// Only add mapping context if not already present
			if (DefaultMappingContext && !Subsystem->HasMappingContext(DefaultMappingContext))
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	// Bind enhanced input actions
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Bind movement
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATFPlayerCharacter::Move);
		}

		// Bind look
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATFPlayerCharacter::Look);
		}

		// Bind jump
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ATFPlayerCharacter::PlayerJump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}

		// Bind perspective toggle
		if (ToggleCameraPerspective)
		{
			EnhancedInputComponent->BindAction(ToggleCameraPerspective, ETriggerEvent::Started, this, &ATFPlayerCharacter::TogglePerspective);
		}

		// Bind sprint
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ATFPlayerCharacter::SprintOn);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATFPlayerCharacter::SprintOff);
		}

		// Bind sneak
		if (SneakAction)
		{
			EnhancedInputComponent->BindAction(SneakAction, ETriggerEvent::Started, this, &ATFPlayerCharacter::SneakOn);
			EnhancedInputComponent->BindAction(SneakAction, ETriggerEvent::Completed, this, &ATFPlayerCharacter::SneakOff);
		}
	}
}

void ATFPlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Get controller rotation
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// Calculate movement direction
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// Add movement input
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ATFPlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Add controller input
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(-LookAxisVector.Y);
	}
}

void ATFPlayerCharacter::SprintOn()
{
	ATFCharacterBase::SetSprinting(true);
}

void ATFPlayerCharacter::SprintOff()
{
	ATFCharacterBase::SetSprinting(false);
}

void ATFPlayerCharacter::SneakOn()
{
	ATFCharacterBase::SetSneaking(true);
}

void ATFPlayerCharacter::SneakOff()
{
	ATFCharacterBase::SetSneaking(false);
}

void ATFPlayerCharacter::PlayerJump()
{
	// Only jump if character can jump and is not falling
	if (ATFCharacterBase::CanCharacterJump() && !GetCharacterMovement()->IsFalling())
	{
		ATFCharacterBase::HasJumped();
	}
}

void ATFPlayerCharacter::TogglePerspective()
{
	// Prevent toggling during active transition
	if (bIsTransitioning)
	{
		return;
	}

	// Toggle perspective state
	bInFirstPerson = !bInFirstPerson;

	// Start smooth transition
	StartCameraTransition(bInFirstPerson);
}

void ATFPlayerCharacter::StartCameraTransition(bool bToFirstPerson)
{
	if (!CameraBoom)
	{
		return;
	}

	// Initialize transition state
	bIsTransitioning = true;
	TransitionTimer = 0.0f;
	TransitionAlpha = 0.0f;

	// Store starting and target camera arm lengths
	StartCameraArmLength = CameraBoom->TargetArmLength;
	TargetCameraArmLength = bToFirstPerson ? 0.0f : ThirdPersonCameraDistance;

	// Store rotation values for smooth transition
	if (Controller)
	{
		StartControlRotation = Controller->GetControlRotation();
		StartCharacterRotation = GetActorRotation();

		if (bToFirstPerson)
		{
			// Transitioning to first person: align character with camera view
			TargetControlRotation = StartControlRotation;
		}
		else
		{
			// Transitioning to third person: maintain current camera direction
			TargetControlRotation = StartControlRotation;
		}
	}

	// Activate both cameras during transition for smooth blending
	if (FirstPersonCamera && ThirdPersonCamera)
	{
		FirstPersonCamera->Activate();
		ThirdPersonCamera->Activate();
	}

	// If smooth transition is disabled, complete immediately
	if (!bSmoothCameraTransition)
	{
		CompleteCameraTransition();
	}
}

void ATFPlayerCharacter::UpdateCameraTransition(float DeltaTime)
{
	if (!bIsTransitioning)
	{
		return;
	}

	// Update transition timer and calculate alpha
	TransitionTimer += DeltaTime;
	TransitionAlpha = FMath::Clamp(TransitionTimer / CameraTransitionDuration, 0.0f, 1.0f);

	// Apply smooth easing curve (ease in-out cubic)
	// Formula: 3t² - 2t³ creates smooth acceleration and deceleration
	float EasedAlpha = TransitionAlpha * TransitionAlpha * (3.0f - 2.0f * TransitionAlpha);

	// Interpolate camera boom arm length
	if (CameraBoom)
	{
		float NewArmLength = FMath::Lerp(StartCameraArmLength, TargetCameraArmLength, EasedAlpha);
		CameraBoom->TargetArmLength = NewArmLength;
	}

	// Apply smooth character rotation during transition if enabled
	if (bSmoothRotationTransition && Controller)
	{
		// Calculate rotation alpha (may complete faster than camera transition)
		float RotationAlpha = FMath::Clamp(TransitionTimer / RotationTransitionDuration, 0.0f, 1.0f);
		float RotationEasedAlpha = RotationAlpha * RotationAlpha * (3.0f - 2.0f * RotationAlpha);

		if (bInFirstPerson)
		{
			// Smoothly rotate character to match camera direction when entering first person
			FRotator NewRotation = UKismetMathLibrary::RLerp(
				StartCharacterRotation,
				FRotator(0.0f, StartControlRotation.Yaw, 0.0f),
				RotationEasedAlpha,
				true
			);
			SetActorRotation(NewRotation);
		}
	}

	// Optional: Apply vignette effect during transition for visual polish
	if (FirstPersonCamera && ThirdPersonCamera)
	{
		if (bInFirstPerson)
		{
			// Fade out third person camera with vignette
			FirstPersonCamera->PostProcessSettings.bOverride_VignetteIntensity = true;
			FirstPersonCamera->PostProcessSettings.VignetteIntensity = FMath::Lerp(0.4f, 0.0f, EasedAlpha);
		}
		else
		{
			// Fade out first person camera with vignette
			ThirdPersonCamera->PostProcessSettings.bOverride_VignetteIntensity = true;
			ThirdPersonCamera->PostProcessSettings.VignetteIntensity = FMath::Lerp(0.4f, 0.0f, EasedAlpha);
		}
	}

	// Check if transition is complete
	if (TransitionAlpha >= 1.0f)
	{
		CompleteCameraTransition();
	}
}

void ATFPlayerCharacter::CompleteCameraTransition()
{
	// Reset transition state
	bIsTransitioning = false;
	TransitionTimer = 0.0f;
	TransitionAlpha = 1.0f;

	// Ensure final camera arm length is set
	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = TargetCameraArmLength;
	}

	// Apply final configuration for the target perspective
	if (bInFirstPerson)
	{
		ConfigureFirstPersonMode();

		// Ensure character is perfectly aligned with camera view at end of transition
		if (Controller && bSmoothRotationTransition)
		{
			FRotator FinalRotation = FRotator(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
			SetActorRotation(FinalRotation);
		}
	}
	else
	{
		ConfigureThirdPersonMode();
	}

	// Clean up post-process effects
	if (FirstPersonCamera)
	{
		FirstPersonCamera->PostProcessSettings.bOverride_VignetteIntensity = false;
	}
	if (ThirdPersonCamera)
	{
		ThirdPersonCamera->PostProcessSettings.bOverride_VignetteIntensity = false;
	}
}

void ATFPlayerCharacter::ConfigureFirstPersonMode()
{
	if (!FirstPersonCamera || !ThirdPersonCamera)
	{
		return;
	}

	// Switch to first person camera
	ThirdPersonCamera->Deactivate();
	FirstPersonCamera->Activate();

	// Configure controller rotation for first person
	// Character rotates instantly with controller yaw (FPS style)
	bUseControllerRotationPitch = false; // Don't pitch entire character
	bUseControllerRotationYaw = true;    // Rotate with mouse/controller
	bUseControllerRotationRoll = false;  // No roll

	// Configure character movement for first person
	if (UCharacterMovementComponent* CharMoveComp = GetCharacterMovement())
	{
		CharMoveComp->bOrientRotationToMovement = false;        // Don't rotate toward movement
		CharMoveComp->bUseControllerDesiredRotation = false;    // Use controller rotation directly
		CharMoveComp->RotationRate = FirstPersonRotationRate;   // Fast rotation for responsiveness
	}
}

void ATFPlayerCharacter::ConfigureThirdPersonMode()
{
	if (!FirstPersonCamera || !ThirdPersonCamera)
	{
		return;
	}

	// Switch to third person camera
	FirstPersonCamera->Deactivate();
	ThirdPersonCamera->Activate();

	// Configure controller rotation for third person
	// Character rotates toward movement direction (action game style)
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;  // Don't rotate with controller
	bUseControllerRotationRoll = false;

	// Configure character movement for third person
	if (UCharacterMovementComponent* CharMoveComp = GetCharacterMovement())
	{
		CharMoveComp->bOrientRotationToMovement = true;         // Rotate toward movement direction
		CharMoveComp->bUseControllerDesiredRotation = false;
		CharMoveComp->RotationRate = ThirdPersonRotationRate;   // Smooth rotation
	}
}