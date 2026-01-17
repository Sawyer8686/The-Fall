// Fill out your copyright notice in the Description page of Project Settings.

#include "TFBaseDoorActor.h"
#include "TFPlayerCharacter.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

ATFBaseDoorActor::ATFBaseDoorActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create door frame mesh
	DoorFrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
	DoorFrameMesh->SetupAttachment(Root);
	DoorFrameMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DoorFrameMesh->SetCollisionResponseToAllChannels(ECR_Block);

	// Create door mesh (moving part)
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	DoorMesh->SetupAttachment(DoorFrameMesh);
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DoorMesh->SetCollisionResponseToAllChannels(ECR_Block);

	// Create audio component
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(DoorMesh);
	AudioComponent->bAutoActivate = false;

	// Set default interaction text
	InteractionText = FText::FromString("Open Door");
	MaxInteractionDistance = 200.0f;

	// Door is reusable (can open/close multiple times)
	bIsReusable = true;
}

void ATFBaseDoorActor::BeginPlay()
{
	Super::BeginPlay();

	// Store initial rotation
	InitialRotation = DoorMesh->GetRelativeRotation();
}

void ATFBaseDoorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update door animation if moving
	if (IsMoving())
	{
		UpdateDoorAnimation(DeltaTime);
	}
}

void ATFBaseDoorActor::UpdateDoorAnimation(float DeltaTime)
{
	if (CurrentAnimationDuration <= 0.0f)
	{
		return;
	}

	// Update timer
	AnimationTimer += DeltaTime;
	float Alpha = FMath::Clamp(AnimationTimer / CurrentAnimationDuration, 0.0f, 1.0f);

	// Smooth easing (ease in-out)
	float EasedAlpha = Alpha * Alpha * (3.0f - 2.0f * Alpha);

	// Interpolate angle
	CurrentAngle = FMath::Lerp(
		DoorState == EDoorState::Opening ? 0.0f : TargetAngle,
		DoorState == EDoorState::Opening ? TargetAngle : 0.0f,
		EasedAlpha
	);

	// Apply rotation
	ApplyDoorRotation(CurrentAngle);

	// Check if animation complete
	if (Alpha >= 1.0f)
	{
		if (DoorState == EDoorState::Opening)
		{
			CompleteOpening();
		}
		else if (DoorState == EDoorState::Closing)
		{
			CompleteClosing();
		}
	}
}

void ATFBaseDoorActor::ApplyDoorRotation(float Angle)
{
	FRotator NewRotation = InitialRotation;

	// Apply angle based on hinge type
	if (HingeType == EDoorHinge::Left)
	{
		NewRotation.Yaw += Angle;
	}
	else
	{
		NewRotation.Yaw -= Angle;
	}

	DoorMesh->SetRelativeRotation(NewRotation);
}

float ATFBaseDoorActor::CalculateTargetAngle(const FVector& PlayerLocation)
{
	// Get door forward vector
	FVector DoorForward = GetActorForwardVector();

	// Get vector from door to player
	FVector ToPlayer = (PlayerLocation - GetActorLocation()).GetSafeNormal();

	// Calculate dot product to determine which side player is on
	float DotProduct = FVector::DotProduct(DoorForward, ToPlayer);

	// If player is on back side and door can't open from both sides, return 0
	if (!bCanOpenFromBothSides && DotProduct < 0.0f)
	{
		return 0.0f;
	}

	// Return angle based on player side
	return MaxOpenAngle;
}

void ATFBaseDoorActor::StartOpening(ATFPlayerCharacter* OpeningCharacter)
{
	if (!OpeningCharacter)
	{
		return;
	}

	// Calculate target angle
	TargetAngle = CalculateTargetAngle(OpeningCharacter->GetActorLocation());

	if (TargetAngle <= 0.0f)
	{
		return;
	}

	// Set state
	DoorState = EDoorState::Opening;
	AnimationTimer = 0.0f;
	CurrentAnimationDuration = OpenDuration;

	// Play sound
	PlayDoorSound(DoorOpenSound);

	// Call blueprint event
	OnDoorStartOpening(OpeningCharacter);
}

void ATFBaseDoorActor::StartClosing()
{
	// Set state
	DoorState = EDoorState::Closing;
	AnimationTimer = 0.0f;
	CurrentAnimationDuration = CloseDuration;

	// Clear auto-close timer
	GetWorld()->GetTimerManager().ClearTimer(AutoCloseTimerHandle);

	// Play sound
	PlayDoorSound(DoorCloseSound);

	// Call blueprint event
	OnDoorStartClosing();
}

void ATFBaseDoorActor::CompleteOpening()
{
	DoorState = EDoorState::Open;
	CurrentAngle = TargetAngle;

	// Set up auto-close if enabled
	if (bAutoClose && AutoCloseDelay > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			AutoCloseTimerHandle,
			this,
			&ATFBaseDoorActor::AutoCloseDoor,
			AutoCloseDelay,
			false
		);
	}

	// Call blueprint event
	OnDoorOpened();
}

void ATFBaseDoorActor::CompleteClosing()
{
	DoorState = EDoorState::Closed;
	CurrentAngle = 0.0f;

	// Call blueprint event
	OnDoorClosed();
}

void ATFBaseDoorActor::PlayDoorSound(USoundBase* Sound)
{
	if (!Sound || !AudioComponent)
	{
		return;
	}

	AudioComponent->SetSound(Sound);
	AudioComponent->Play();
}

void ATFBaseDoorActor::AutoCloseDoor()
{
	if (IsOpen())
	{
		CloseDoor();
	}
}

bool ATFBaseDoorActor::IsPlayerOnCorrectSide(const FVector& PlayerLocation) const
{
	if (bCanOpenFromBothSides)
	{
		return true;
	}

	FVector DoorForward = GetActorForwardVector();
	FVector ToPlayer = (PlayerLocation - GetActorLocation()).GetSafeNormal();
	float DotProduct = FVector::DotProduct(DoorForward, ToPlayer);

	return DotProduct >= 0.0f;
}

bool ATFBaseDoorActor::Interact_Implementation(APawn* Instigator)
{
	ATFPlayerCharacter* InstigatorCharacter = Cast<ATFPlayerCharacter>(Instigator);
	if (!InstigatorCharacter)
	{
		return false;
	}

	// Key door logic
	if (bRequiresKey)
	{
		bool bHasKey = CharacterHasKey(InstigatorCharacter);

		// Case 1: Door is locked
		if (bIsLocked)
		{
			if (bHasKey)
			{
				// Player has key - unlock the door
				return UnlockDoor(InstigatorCharacter);
			}
			else
			{
				// Player doesn't have key - show "locked" feedback
				PlayDoorSound(DoorLockedSound);
				OnKeyRequired(InstigatorCharacter);
				OnDoorLocked(InstigatorCharacter);
				return false;
			}
		}

		// Case 2: Door is unlocked and closed - open it
		if (IsClosed())
		{
			return OpenDoor(InstigatorCharacter);
		}

		// Case 3: Door is unlocked and open - close it
		if (IsOpen())
		{
			return CloseDoor();
		}

		// Door is moving, do nothing
		return false;
	}

	// Regular door (no key required) - just toggle
	return ToggleDoor(InstigatorCharacter);
}

FInteractionData ATFBaseDoorActor::GetInteractionData_Implementation(APawn* Instigator) const
{
	FInteractionData Data = Super::GetInteractionData_Implementation(Instigator);

	ATFPlayerCharacter* InstigatorCharacter = Cast<ATFPlayerCharacter>(Instigator);

	// Key door logic
	if (bRequiresKey)
	{
		bool bHasKey = CharacterHasKey(InstigatorCharacter);

		// Locked door states
		if (bIsLocked)
		{
			if (bHasKey)
			{
				// Player has the key - show unlock option
				Data.InteractionText = FText::FromString("Unlock Door");
				Data.SecondaryText = FText::Format(
					FText::FromString("Using {0}"),
					RequiredKeyName
				);
				Data.bCanInteract = true;
			}
			else
			{
				// Player doesn't have the key - show requirement
				Data.InteractionText = FText::FromString("Locked");
				Data.SecondaryText = FText::Format(
					FText::FromString("Requires {0}"),
					RequiredKeyName
				);
				Data.bCanInteract = false;
			}
		}
		// Unlocked door states
		else
		{
			if (IsClosed())
			{
				Data.InteractionText = FText::FromString("Open Door");
				Data.bCanInteract = true;
			}
			else if (IsOpen())
			{
				Data.InteractionText = FText::FromString("Close Door");
				Data.bCanInteract = true;
			}
			else
			{
				// Door is moving
				Data.InteractionText = FText::FromString("Wait...");
				Data.bCanInteract = false;
			}
		}

		return Data;
	}

	// Regular door (no key required)
	if (IsClosed())
	{
		Data.InteractionText = FText::FromString("Open Door");
	}
	else if (IsOpen())
	{
		Data.InteractionText = FText::FromString("Close Door");
	}
	else
	{
		Data.InteractionText = FText::FromString("Wait...");
		Data.bCanInteract = false;
	}

	return Data;
}

bool ATFBaseDoorActor::CanInteract_Implementation(APawn* Instigator) const
{
	if (!Super::CanInteract_Implementation(Instigator))
	{
		return false;
	}

	// Can't interact while door is moving
	if (IsMoving())
	{
		return false;
	}

	// Check if player is on correct side (if door has restrictions)
	if (Instigator && !IsPlayerOnCorrectSide(Instigator->GetActorLocation()))
	{
		return false;
	}

	return true;
}

bool ATFBaseDoorActor::OpenDoor(ATFPlayerCharacter* OpeningCharacter)
{
	// Check if can open
	if (IsOpen() || IsMoving() || IsDoorLocked())
	{
		return false;
	}

	// Start opening
	StartOpening(OpeningCharacter);
	return true;
}

bool ATFBaseDoorActor::CloseDoor()
{
	// Check if can close
	if (IsClosed() || IsMoving())
	{
		return false;
	}

	// Start closing
	StartClosing();
	return true;
}

bool ATFBaseDoorActor::ToggleDoor(ATFPlayerCharacter* TogglingCharacter)
{
	if (IsClosed())
	{
		return OpenDoor(TogglingCharacter);
	}
	else if (IsOpen())
	{
		return CloseDoor();
	}

	return false;
}

float ATFBaseDoorActor::GetDoorOpenPercentage() const
{
	if (MaxOpenAngle <= 0.0f)
	{
		return 0.0f;
	}

	return CurrentAngle / MaxOpenAngle;
}

// ============================================================================
// Key Door Functions
// ============================================================================

bool ATFBaseDoorActor::IsDoorLocked_Implementation() const
{
	return bRequiresKey && bIsLocked;
}

bool ATFBaseDoorActor::UnlockDoor_Implementation(ATFPlayerCharacter* UnlockingCharacter)
{
	// If no key required, door is always "unlocked"
	if (!bRequiresKey)
	{
		return true;
	}

	// Check if character has the required key
	if (!CharacterHasKey(UnlockingCharacter))
	{
		return false;
	}

	// Already unlocked
	if (!bIsLocked)
	{
		return false;
	}

	// Unlock the door
	bIsLocked = false;

	// Play unlock sound
	PlayDoorSound(DoorUnlockSound);

	// Call blueprint event
	OnDoorUnlocked(UnlockingCharacter);

	UE_LOG(LogTemp, Log, TEXT("ATFBaseDoorActor: Door unlocked with key '%s'"), *RequiredKeyID.ToString());

	return true;
}

bool ATFBaseDoorActor::LockDoor(ATFPlayerCharacter* LockingCharacter)
{
	// Can only lock key doors
	if (!bRequiresKey)
	{
		return false;
	}

	// Check if relocking is allowed
	if (!bCanRelock)
	{
		return false;
	}

	// Check if character has the required key
	if (!CharacterHasKey(LockingCharacter))
	{
		return false;
	}

	// Already locked
	if (bIsLocked)
	{
		return false;
	}

	// Door must be closed to lock
	if (!IsClosed())
	{
		return false;
	}

	// Lock the door
	bIsLocked = true;

	// Play lock sound
	PlayDoorSound(DoorLockSound);

	// Call blueprint event
	OnDoorRelocked(LockingCharacter);

	UE_LOG(LogTemp, Log, TEXT("ATFBaseDoorActor: Door locked with key '%s'"), *RequiredKeyID.ToString());

	return true;
}

void ATFBaseDoorActor::SetLockedState(bool bNewLockState)
{
	bIsLocked = bNewLockState;
}

bool ATFBaseDoorActor::CharacterHasKey(const ATFPlayerCharacter* Character) const
{
	if (!Character || !bRequiresKey || RequiredKeyID.IsNone())
	{
		return false;
	}

	return Character->HasKey(RequiredKeyID);
}