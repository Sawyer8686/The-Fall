// Fill out your copyright notice in the Description page of Project Settings.

#include "TFBaseDoorActor.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

ATFBaseDoorActor::ATFBaseDoorActor()
{
	PrimaryActorTick.bCanEverTick = true;

	DoorFrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
	DoorFrameMesh->SetupAttachment(Root);
	DoorFrameMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DoorFrameMesh->SetCollisionResponseToAllChannels(ECR_Block);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	DoorMesh->SetupAttachment(DoorFrameMesh);
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DoorMesh->SetCollisionResponseToAllChannels(ECR_Block);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(DoorMesh);
	AudioComponent->bAutoActivate = false;

	InteractionText = FText::FromString("Open Door");
	MaxInteractionDistance = 200.0f;

	bIsReusable = true;
}

void ATFBaseDoorActor::BeginPlay()
{
	Super::BeginPlay();

	InitialRotation = DoorMesh->GetRelativeRotation();
}

void ATFBaseDoorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

	AnimationTimer += DeltaTime;
	float Alpha = FMath::Clamp(AnimationTimer / CurrentAnimationDuration, 0.0f, 1.0f);

	float EasedAlpha = Alpha * Alpha * (3.0f - 2.0f * Alpha);

	CurrentAngle = FMath::Lerp(
		DoorState == EDoorState::Opening ? 0.0f : TargetAngle,
		DoorState == EDoorState::Opening ? TargetAngle : 0.0f,
		EasedAlpha
	);

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
	FVector DoorForward = GetActorForwardVector();

	FVector ToPlayer = (PlayerLocation - GetActorLocation()).GetSafeNormal();

	float DotProduct = FVector::DotProduct(DoorForward, ToPlayer);

	if (!bCanOpenFromBothSides && DotProduct < 0.0f)
	{
		return 0.0f;
	}

	return MaxOpenAngle;
}

void ATFBaseDoorActor::StartOpening(APawn* OpeningCharacter)
{
	if (!OpeningCharacter)
	{
		return;
	}

	TargetAngle = CalculateTargetAngle(OpeningCharacter->GetActorLocation());

	if (TargetAngle <= 0.0f)
	{
		return;
	}

	DoorState = EDoorState::Opening;
	AnimationTimer = 0.0f;
	CurrentAnimationDuration = OpenDuration;

	PlayDoorSound(DoorOpenSound);

	OnDoorStartOpening(OpeningCharacter);
}

void ATFBaseDoorActor::StartClosing()
{
	DoorState = EDoorState::Closing;
	AnimationTimer = 0.0f;
	CurrentAnimationDuration = CloseDuration;

	GetWorld()->GetTimerManager().ClearTimer(AutoCloseTimerHandle);

	PlayDoorSound(DoorCloseSound);

	OnDoorStartClosing();
}

void ATFBaseDoorActor::CompleteOpening()
{
	DoorState = EDoorState::Open;
	CurrentAngle = TargetAngle;

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

	OnDoorOpened();
}

void ATFBaseDoorActor::CompleteClosing()
{
	DoorState = EDoorState::Closed;
	CurrentAngle = 0.0f;

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

bool ATFBaseDoorActor::Interact_Implementation(APawn* InstigatorPawn)
{
	if (!InstigatorPawn)
	{
		return false;
	}

	if (bRequiresKey)
	{
		bool bHasKey = CharacterHasKey(InstigatorPawn);

		// Case 1: Door is locked
		if (bIsLocked)
		{
			if (bHasKey)
			{
				return UnlockDoor(InstigatorPawn);
			}
			else
			{
				PlayDoorSound(DoorLockedSound);
				OnKeyRequired(InstigatorPawn);
				OnDoorLocked(InstigatorPawn);
				return false;
			}
		}

		if (IsClosed())
		{
			return OpenDoor(InstigatorPawn);
		}

		if (IsOpen())
		{
			return CloseDoor();
		}

		return false;
	}

	return ToggleDoor(InstigatorPawn);
}

FInteractionData ATFBaseDoorActor::GetInteractionData_Implementation(APawn* InstigatorPawn) const
{
	FInteractionData Data = Super::GetInteractionData_Implementation(InstigatorPawn);

	if (bRequiresKey)
	{
		bool bHasKey = CharacterHasKey(InstigatorPawn);

		if (bIsLocked)
		{
			if (bHasKey)
			{
				Data.InteractionText = FText::FromString("Unlock Door");
				Data.SecondaryText = FText::Format(
					FText::FromString("Using {0}"),
					RequiredKeyName
				);
				Data.bCanInteract = true;
			}
			else
			{
				Data.InteractionText = FText::FromString("Locked");
				Data.SecondaryText = FText::Format(
					FText::FromString("Requires {0}"),
					RequiredKeyName
				);
				Data.bCanInteract = false;
			}
		}
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
				Data.InteractionText = FText::FromString("Wait...");
				Data.bCanInteract = false;
			}
		}

		return Data;
	}

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

bool ATFBaseDoorActor::CanInteract_Implementation(APawn* InstigatorPawn) const
{
	if (!Super::CanInteract_Implementation(InstigatorPawn))
	{
		return false;
	}

	if (IsMoving())
	{
		return false;
	}

	if (InstigatorPawn && !IsPlayerOnCorrectSide(InstigatorPawn->GetActorLocation()))
	{
		return false;
	}

	return true;
}

bool ATFBaseDoorActor::OpenDoor(APawn* OpeningCharacter)
{
	if (IsOpen() || IsMoving() || IsDoorLocked())
	{
		return false;
	}

	StartOpening(OpeningCharacter);
	return true;
}

bool ATFBaseDoorActor::CloseDoor()
{
	if (IsClosed() || IsMoving())
	{
		return false;
	}

	StartClosing();
	return true;
}

bool ATFBaseDoorActor::ToggleDoor(APawn* TogglingCharacter)
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

bool ATFBaseDoorActor::IsDoorLocked_Implementation() const
{
	return bRequiresKey && bIsLocked;
}

bool ATFBaseDoorActor::UnlockDoor_Implementation(APawn* UnlockingCharacter)
{
	if (!bRequiresKey)
	{
		return true;
	}

	if (!CharacterHasKey(UnlockingCharacter))
	{
		return false;
	}

	if (!bIsLocked)
	{
		return false;
	}

	bIsLocked = false;

	PlayDoorSound(DoorUnlockSound);

	OnDoorUnlocked(UnlockingCharacter);

	UE_LOG(LogTemp, Log, TEXT("ATFBaseDoorActor: Door unlocked with key '%s'"), *RequiredKeyID.ToString());

	return true;
}

bool ATFBaseDoorActor::LockDoor(APawn* LockingCharacter)
{
	if (!bRequiresKey)
	{
		return false;
	}

	if (!bCanRelock)
	{
		return false;
	}

	if (!CharacterHasKey(LockingCharacter))
	{
		return false;
	}

	if (bIsLocked)
	{
		return false;
	}

	if (!IsClosed())
	{
		return false;
	}

	bIsLocked = true;

	PlayDoorSound(DoorLockSound);

	OnDoorRelocked(LockingCharacter);

	UE_LOG(LogTemp, Log, TEXT("ATFBaseDoorActor: Door locked with key '%s'"), *RequiredKeyID.ToString());

	return true;
}

void ATFBaseDoorActor::SetLockedState(bool bNewLockState)
{
	bIsLocked = bNewLockState;
}

bool ATFBaseDoorActor::CharacterHasKey(const APawn* Character) const
{
	if (!Character || !bRequiresKey || RequiredKeyID.IsNone())
	{
		return false;
	}

	if (Character->Implements<UTFKeyHolderInterface>())
	{
		return ITFKeyHolderInterface::Execute_HasKey(Character, RequiredKeyID);
	}

	return false;
}

bool ATFBaseDoorActor::ToggleLock_Implementation(APawn* Character)
{
	if (!bRequiresKey)
	{
		return false;
	}

	if (!Character)
	{
		return false;
	}

	if (!IsClosed() || IsMoving())
	{
		return false;
	}

	if (bIsLocked)
	{
		return UnlockDoor(Character);
	}

	return LockDoor(Character);
}