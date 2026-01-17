// Fill out your copyright notice in the Description page of Project Settings.

#include "TFPlayerCharacter.h"
#include "Components/TFStaminaComponent.h"
#include "Components/TFInteractionComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


ATFPlayerCharacter::ATFPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	StaminaComponent = CreateDefaultSubobject<UTFStaminaComponent>(TEXT("StaminaComponent"));
	InteractionComponent = CreateDefaultSubobject<UTFInteractionComponent>(TEXT("InteractionComponent"));

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), "head");
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->SetRelativeLocation(FirstPersonCameraOffset);
	FirstPersonCamera->SetRelativeRotation(FirstPersonCameraRotation);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void ATFPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	BindStaminaEvents();
}

void ATFPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext && !Subsystem->HasMappingContext(DefaultMappingContext))
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATFPlayerCharacter::Move);
		}

		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATFPlayerCharacter::Look);
		}

		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ATFPlayerCharacter::PlayerJump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}

		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ATFPlayerCharacter::SprintOn);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATFPlayerCharacter::SprintOff);
		}

		if (SneakAction)
		{
			EnhancedInputComponent->BindAction(SneakAction, ETriggerEvent::Started, this, &ATFPlayerCharacter::SneakOn);
			EnhancedInputComponent->BindAction(SneakAction, ETriggerEvent::Completed, this, &ATFPlayerCharacter::SneakOff);
		}

		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ATFPlayerCharacter::InteractPressed);
		}
	}
}

void ATFPlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ATFPlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
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
	if (CanCharacterJump() && !GetCharacterMovement()->IsFalling())
	{
		HasJumped();
	}
}

void ATFPlayerCharacter::InteractPressed()
{
	if (InteractionComponent)
	{
		InteractionComponent->Interact();
	}
}

void ATFPlayerCharacter::SetSprinting(const bool bSprinting)
{
	if (bSprinting)
	{
		if (!StaminaComponent || !StaminaComponent->CanSprint())
		{
			return;
		}

		if (IsSneaking())
		{
			return;
		}

		bIsSprinting = true;

		if (StaminaComponent)
		{
			StaminaComponent->StartStaminaDrain(StaminaComponent->SprintDrainRate);
		}
	}
	else
	{
		if (!bIsSprinting)
		{
			return;
		}

		bIsSprinting = false;

		if (StaminaComponent)
		{
			StaminaComponent->StopStaminaDrain();
		}
	}
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
	if (bIsSprinting)
	{
		SetSprinting(false);
	}

	UpdateMovementSpeed();

	OnStaminaDepleted();
}

void ATFPlayerCharacter::HandleStaminaRecovered()
{
	UpdateMovementSpeed();

	OnStaminaRecovered();
}

void ATFPlayerCharacter::OnStaminaDepleted_Implementation()
{
}

void ATFPlayerCharacter::OnStaminaRecovered_Implementation()
{
}

bool ATFPlayerCharacter::CanCharacterJump() const
{
	if (!ATFCharacterBase::CanCharacterJump())
	{
		return false;
	}

	if (StaminaComponent)
	{
		return StaminaComponent->CanJump();
	}

	return true;
}

void ATFPlayerCharacter::HasJumped()
{
	if (StaminaComponent)
	{
		StaminaComponent->ConsumeStamina(
			StaminaComponent->JumpStaminaCost,
			EStaminaDrainReason::Jump
		);
	}
	ATFCharacterBase::HasJumped();
}

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