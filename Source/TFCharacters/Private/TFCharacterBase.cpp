#include "TFCharacterBase.h"

#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ATFCharacterBase::ATFCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATFCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
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

float ATFCharacterBase::GetSprintSpeed() const
{
	return SprintSpeed;
}

void ATFCharacterBase::SetSprinting(const bool bSprinting)
{
	if(bSprinting)
	{
		bIsSneaking = false;
		GetCharacterMovement()->MaxWalkSpeed = GetSprintSpeed();	
		bIsSprinting = true;
		return;
	}

	if(bIsSneaking)
	{
		return;
	}
	
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = GetWalkSpeed();
	return;
    
}

void ATFCharacterBase::SetSneaking(const bool bSneaking)
{
	if(bSneaking)
	{
		bIsSprinting = false;
		GetCharacterMovement()->MaxWalkSpeed = GetSneakSpeed();	
		bIsSneaking = true;
		return;
	}

	if(bIsSprinting)
	{
		return;
	}

	bIsSneaking = false;
	GetCharacterMovement()->MaxWalkSpeed = GetWalkSpeed();
	return;
}

// Called every frame
void ATFCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ATFCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

