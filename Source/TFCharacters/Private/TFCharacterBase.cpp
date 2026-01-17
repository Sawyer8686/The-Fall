// Fill out your copyright notice in the Description page of Project Settings.

#include "TFCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

ATFCharacterBase::ATFCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool ATFCharacterBase::CanCharacterJump() const
{
	return CanJump();
}

void ATFCharacterBase::HasJumped()
{
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
	if (bIsSneaking == bSneaking)
	{
		return;
	}

	bIsSneaking = bSneaking;
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

