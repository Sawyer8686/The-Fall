// Fill out your copyright notice in the Description page of Project Settings.


#include "TFPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

ATFPlayerCharacter::ATFPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = false;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;	
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	ThirdPersonCamera->bUsePawnControlRotation = false;
	ThirdPersonCamera->Deactivate();
	ThirdPersonCamera->bAutoActivate = false;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), "head");
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->Activate();
	FirstPersonCamera->SetRelativeLocation(FVector(15, 20, 2.5));
	FirstPersonCamera->SetRelativeRotation(FRotator(0.f, -90.f, 90.f));


}

void ATFPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ATFPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}	

void ATFPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if(APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
	          Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
		
	}

	if(UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATFPlayerCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATFPlayerCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ATFPlayerCharacter::PlayerJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(ToggleCameraPerspective, ETriggerEvent::Started, this, &ATFPlayerCharacter::TogglePerspective);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ATFPlayerCharacter::SprintOn);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATFPlayerCharacter::SprintOff);
		EnhancedInputComponent->BindAction(SneakAction, ETriggerEvent::Started, this, &ATFPlayerCharacter::SneakOn);
		EnhancedInputComponent->BindAction(SneakAction, ETriggerEvent::Completed, this, &ATFPlayerCharacter::SneakOff);
		
	}

}

void ATFPlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	if(Controller != nullptr)
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
	if(Controller != nullptr)
	{
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
	if (ATFCharacterBase::CanCharacterJump() && !GetCharacterMovement()->IsFalling())
	{
		ATFCharacterBase::HasJumped();
	}
}

void ATFPlayerCharacter::TogglePerspective()
{
	bInFirstPerson = !bInFirstPerson;
	if(!bInFirstPerson)
	{
		FirstPersonCamera->Deactivate();
		ThirdPersonCamera->Activate();
		bUseControllerRotationPitch = false;
		bUseControllerRotationYaw = false;
		bUseControllerRotationRoll = false;
		return;
	}
	
	
	ThirdPersonCamera->Deactivate();
	FirstPersonCamera->Activate();
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	return;
	
}



