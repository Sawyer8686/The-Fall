// Fill out your copyright notice in the Description page of Project Settings.

#include "TFCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

ATFCharacterBase::ATFCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATFCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

bool ATFCharacterBase::CanCharacterJump() const
{
	// Base jump check - can be overridden by derived classes for stamina/conditions
	return CanJump();
}

void ATFCharacterBase::HasJumped()
{
	// Base jump execution - override in derived classes to add stamina consumption
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

void ATFCharacterBase::SetSneaking(const bool bSneaking)
{
	if (bSneaking)
	{
		bIsSneaking = true;
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

void ATFCharacterBase::UpdateMovementSpeed()
{
	if (!GetCharacterMovement())
	{
		return;
	}

	float TargetSpeed = bIsSneaking ? SneakSpeed : WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
}

void ATFCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATFCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}