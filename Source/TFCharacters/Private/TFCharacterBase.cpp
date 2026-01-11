// Fill out your copyright notice in the Description page of Project Settings.

#include "TFCharacterBase.h"
#include "TFStaminaComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ATFCharacterBase::ATFCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create stamina component
	StaminaComponent = CreateDefaultSubobject<UTFStaminaComponent>(TEXT("StaminaComponent"));
}

void ATFCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// Bind to stamina events
	BindStaminaEvents();
}

void ATFCharacterBase::BindStaminaEvents()
{
	if (StaminaComponent)
	{
		StaminaComponent->OnStaminaDepleted.AddDynamic(this, &ATFCharacterBase::HandleStaminaDepleted);
		StaminaComponent->OnStaminaRecovered.AddDynamic(this, &ATFCharacterBase::HandleStaminaRecovered);
	}
}

void ATFCharacterBase::HandleStaminaDepleted()
{
	// Force stop sprinting when stamina depleted
	if (bIsSprinting)
	{
		SetSprinting(false);
	}

	// Update movement speed for exhaustion penalty
	UpdateMovementSpeed();

	// Call blueprint event
	OnStaminaDepleted();
}

void ATFCharacterBase::HandleStaminaRecovered()
{
	// Remove exhaustion penalty
	UpdateMovementSpeed();

	// Call blueprint event
	OnStaminaRecovered();
}

void ATFCharacterBase::UpdateMovementSpeed()
{
	if (!GetCharacterMovement() || !StaminaComponent)
	{
		return;
	}

	float TargetSpeed = WalkSpeed;

	if (bIsSprinting)
	{
		TargetSpeed = SprintSpeed;
	}
	else if (bIsSneaking)
	{
		TargetSpeed = SneakSpeed;
	}

	// Apply exhaustion penalty if exhausted
	if (StaminaComponent->IsExhausted())
	{
		TargetSpeed *= ExhaustedSpeedMultiplier;
	}

	GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
}

bool ATFCharacterBase::CanCharacterJump() const
{
	// Check base jump conditions and stamina
	if (!CanJump())
	{
		return false;
	}

	if (StaminaComponent)
	{
		return StaminaComponent->CanJump();
	}

	return true;
}

void ATFCharacterBase::HasJumped()
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
	ACharacter::Jump();
}

float ATFCharacterBase::GetSneakSpeed() const
{
	return SneakSpeed;
}

float ATFCharacterBase::GetWalkSpeed() const
{
	return WalkSpeed;
}

float ATFCharacterBase::GetSprintSpeed() const
{
	return SprintSpeed;
}

void ATFCharacterBase::SetSprinting(const bool bSprinting)
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
		if (bIsSneaking)
		{
			return;
		}

		bIsSprinting = true;
		bIsSneaking = false;

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

void ATFCharacterBase::SetSneaking(const bool bSneaking)
{
	if (bSneaking)
	{
		// Can't sneak while sprinting
		if (bIsSprinting)
		{
			return;
		}

		bIsSneaking = true;
		bIsSprinting = false;
	}
	else
	{
		if (!bIsSneaking)
		{
			return;
		}

		bIsSneaking = false;
	}

	// Update movement speed
	UpdateMovementSpeed();
}

void ATFCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATFCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}