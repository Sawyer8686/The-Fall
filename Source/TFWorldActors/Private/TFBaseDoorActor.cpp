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

bool ATFBaseDoorActor::Interact_Implementation(ATFPlayerCharacter* InstigatorCharacter)
{
	if (!InstigatorCharacter)
	{
		return false;
	}

	// Check if locked
	if (IsDoorLocked())
	{
		PlayDoorSound(DoorLockedSound);
		OnDoorLocked(InstigatorCharacter);
		return false;
	}

	// Toggle door
	return ToggleDoor(InstigatorCharacter);
}

FInteractionData ATFBaseDoorActor::GetInteractionData_Implementation(ATFPlayerCharacter* InstigatorCharacter) const
{
	FInteractionData Data = Super::GetInteractionData_Implementation(InstigatorCharacter);

	// Update text based on door state
	if (IsDoorLocked())
	{
		Data.InteractionText = FText::FromString("Locked");
		Data.bCanInteract = false;
	}
	else if (IsClosed())
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

bool ATFBaseDoorActor::CanInteract_Implementation(ATFPlayerCharacter* InstigatorCharacter) const
{
	if (!Super::CanInteract_Implementation(InstigatorCharacter))
	{
		return false;
	}

	// Can't interact while door is moving
	if (IsMoving())
	{
		return false;
	}

	// Check if player is on correct side (if door has restrictions)
	if (InstigatorCharacter && !IsPlayerOnCorrectSide(InstigatorCharacter->GetActorLocation()))
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