// Fill out your copyright notice in the Description page of Project Settings.

#include "TFPlayerCharacter.h"
#include "TFStaminaComponent.h"
#include "Components/TFInteractionComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


ATFPlayerCharacter::ATFPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create stamina component (player only)
	StaminaComponent = CreateDefaultSubobject<UTFStaminaComponent>(TEXT("StaminaComponent"));

	// Create interaction component
	InteractionComponent = CreateDefaultSubobject<UTFInteractionComponent>(TEXT("InteractionComponent"));

	// Capsule component setup
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Character movement configuration
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// First person camera setup (attached to head socket)
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), "head");
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->SetRelativeLocation(FirstPersonCameraOffset);
	FirstPersonCamera->SetRelativeRotation(FirstPersonCameraRotation);

	// Configure for first person mode
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void ATFPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Bind to stamina events
	BindStaminaEvents();
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

		// Bind interact
		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ATFPlayerCharacter::InteractPressed);
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &ATFPlayerCharacter::InteractReleased);
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
	SetSprinting(true);
}

void ATFPlayerCharacter::SprintOff()
{
	SetSprinting(false);
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
	if (CanCharacterJump() && !GetCharacterMovement()->IsFalling())
	{
		HasJumped();
	}
}

void ATFPlayerCharacter::InteractPressed()
{
	if (InteractionComponent)
	{
		// Start hold interaction (will handle instant interactions automatically)
		InteractionComponent->StartHoldInteraction();
	}
}

void ATFPlayerCharacter::InteractReleased()
{
	if (InteractionComponent)
	{
		// Stop hold interaction
		InteractionComponent->StopHoldInteraction();
	}
}

void ATFPlayerCharacter::SetSprinting(const bool bSprinting)
{
	// Check if can sprint
	if (bSprinting)
	{
		// Can't sprint if exhausted or low stamina
		if (!StaminaComponent || !StaminaComponent->CanSprint())
		{
			return;
		}

		// Can't sprint while sneaking
		if (IsSneaking())
		{
			return;
		}

		bIsSprinting = true;

		// Start stamina drain
		if (StaminaComponent)
		{
			StaminaComponent->StartStaminaDrain(StaminaComponent->SprintDrainRate);
		}
	}
	else
	{
		// Stop sprinting
		if (!bIsSprinting)
		{
			return;
		}

		bIsSprinting = false;

		// Stop stamina drain
		if (StaminaComponent)
		{
			StaminaComponent->StopStaminaDrain();
		}
	}

	// Update movement speed
	UpdateMovementSpeed();
}

float ATFPlayerCharacter::GetSprintSpeed() const
{
	return SprintSpeed;
}

void ATFPlayerCharacter::UpdateMovementSpeed()
{
	if (!GetCharacterMovement())
	{
		return;
	}

	float TargetSpeed = GetWalkSpeed();

	if (bIsSprinting)
	{
		TargetSpeed = SprintSpeed;
	}
	else if (IsSneaking())
	{
		TargetSpeed = GetSneakSpeed();
	}

	// Apply exhaustion penalty if exhausted
	if (StaminaComponent && StaminaComponent->IsExhausted())
	{
		TargetSpeed *= ExhaustedSpeedMultiplier;
	}

	GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
}

void ATFPlayerCharacter::BindStaminaEvents()
{
	if (StaminaComponent)
	{
		StaminaComponent->OnStaminaDepleted.AddDynamic(this, &ATFPlayerCharacter::HandleStaminaDepleted);
		StaminaComponent->OnStaminaRecovered.AddDynamic(this, &ATFPlayerCharacter::HandleStaminaRecovered);
	}
}

void ATFPlayerCharacter::HandleStaminaDepleted()
{
	// Force stop sprinting when stamina depleted
	if (bIsSprinting)
	{
		SetSprinting(false);
	}

	// Update movement speed for exhaustion penalty
	UpdateMovementSpeed();

	// Call blueprint native event
	OnStaminaDepleted();
}

void ATFPlayerCharacter::HandleStaminaRecovered()
{
	// Remove exhaustion penalty
	UpdateMovementSpeed();

	// Call blueprint native event
	OnStaminaRecovered();
}

void ATFPlayerCharacter::OnStaminaDepleted_Implementation()
{
	// Default C++ implementation (can be overridden in Blueprint)
	// Add any default behavior here (audio, VFX, etc.)
}

void ATFPlayerCharacter::OnStaminaRecovered_Implementation()
{
	// Default C++ implementation (can be overridden in Blueprint)
	// Add any default behavior here
}

bool ATFPlayerCharacter::CanCharacterJump() const
{
	// Check base jump conditions
	if (!ATFCharacterBase::CanCharacterJump())
	{
		return false;
	}

	// Check stamina
	if (StaminaComponent)
	{
		return StaminaComponent->CanJump();
	}

	return true;
}

void ATFPlayerCharacter::HasJumped()
{
	// Consume stamina for jump
	if (StaminaComponent)
	{
		StaminaComponent->ConsumeStamina(
			StaminaComponent->JumpStaminaCost,
			EStaminaDrainReason::Jump
		);
	}

	// Execute jump
	ATFCharacterBase::HasJumped();
}

// ============================================================================
 // Key Collection
 // ============================================================================

bool ATFPlayerCharacter::HasKey(FName KeyID) const
{
	return CollectedKeys.Contains(KeyID);
}

void ATFPlayerCharacter::AddKey(FName KeyID)
{
	if (KeyID.IsNone())
	{
		return;
	}

	bool bWasAlreadyInSet = false;
	CollectedKeys.Add(KeyID, &bWasAlreadyInSet);

	if (!bWasAlreadyInSet)
	{
		OnKeyAdded(KeyID);
		UE_LOG(LogTemp, Log, TEXT("Key added: %s"), *KeyID.ToString());
	}
}

bool ATFPlayerCharacter::RemoveKey(FName KeyID)
{
	if (KeyID.IsNone())
	{
		return false;
	}

	int32 NumRemoved = CollectedKeys.Remove(KeyID);

	if (NumRemoved > 0)
	{
		OnKeyRemoved(KeyID);
		UE_LOG(LogTemp, Log, TEXT("Key removed: %s"), *KeyID.ToString());
		return true;
	}

	return false;
}