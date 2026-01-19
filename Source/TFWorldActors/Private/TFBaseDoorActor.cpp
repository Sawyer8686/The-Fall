// Fill out your copyright notice in the Description page of Project Settings.

#include "TFBaseDoorActor.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

ATFBaseDoorActor::ATFBaseDoorActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

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

	MaxInteractionDistance = 200.0f;

	bIsReusable = true;
}

void ATFBaseDoorActor::BeginPlay()
{
	Super::BeginPlay();

	InitialRotation = DoorMesh->GetRelativeRotation();
}

void ATFBaseDoorActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoCloseTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
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

	SetActorTickEnabled(true);

	PlayDoorSound(DoorOpenSound);
	PlayDoorMovementSound();

	OnDoorStartOpening(OpeningCharacter);
}

void ATFBaseDoorActor::StartClosing()
{
	DoorState = EDoorState::Closing;
	AnimationTimer = 0.0f;
	CurrentAnimationDuration = CloseDuration;

	SetActorTickEnabled(true);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoCloseTimerHandle);
	}

	PlayDoorSound(DoorCloseSound);
	PlayDoorMovementSound();

	OnDoorStartClosing();
}

void ATFBaseDoorActor::CompleteOpening()
{
	DoorState = EDoorState::Open;
	CurrentAngle = TargetAngle;

	SetActorTickEnabled(false);
	StopDoorMovementSound();

	if (bAutoClose && AutoCloseDelay > 0.0f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				AutoCloseTimerHandle,
				this,
				&ATFBaseDoorActor::AutoCloseDoor,
				AutoCloseDelay,
				false
			);
		}
	}

	OnDoorOpened();
}

void ATFBaseDoorActor::CompleteClosing()
{
	DoorState = EDoorState::Closed;
	CurrentAngle = 0.0f;

	SetActorTickEnabled(false);
	StopDoorMovementSound();

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

void ATFBaseDoorActor::PlayDoorMovementSound()
{
	if (!DoorMovementSound || !AudioComponent)
	{
		return;
	}

	if (AudioComponent->Sound != DoorMovementSound || !AudioComponent->IsPlaying())
	{
		AudioComponent->SetSound(DoorMovementSound);
		AudioComponent->Play();
	}
}

void ATFBaseDoorActor::StopDoorMovementSound()
{
	if (AudioComponent && AudioComponent->Sound == DoorMovementSound && AudioComponent->IsPlaying())
	{
		AudioComponent->Stop();
	}
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

bool ATFBaseDoorActor::Interact(APawn* InstigatorPawn)
{
	if (!InstigatorPawn)
	{
		return false;
	}

	if (IsMoving())
	{
		return false;
	}

	if (bRequiresKey && bIsLocked)
	{
		PlayDoorSound(DoorLockedSound);
		OnKeyRequired(InstigatorPawn);
		OnDoorLocked(InstigatorPawn);
		return false;
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

FInteractionData ATFBaseDoorActor::GetInteractionData(APawn* InstigatorPawn) const
{
	FInteractionData Data = Super::GetInteractionData(InstigatorPawn);

	if (bRequiresKey)
	{
		bool bHasKey = CharacterHasKey(InstigatorPawn);

		if (bIsLocked)
		{
			if (bHasKey)
			{
				Data.bCanInteract = true;
			}
			else
			{
				Data.bCanInteract = false;
			}
		}
		else
		{
			if (IsClosed())
			{
				Data.bCanInteract = true;
			}
			else if (IsOpen())
			{
				Data.bCanInteract = true;
			}
			else
			{
				Data.bCanInteract = false;
			}
		}

		return Data;
	}

	else
	{
		Data.bCanInteract = false;
	}

	return Data;
}

bool ATFBaseDoorActor::CanInteract(APawn* InstigatorPawn) const
{
	if (!Super::CanInteract(InstigatorPawn))
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

bool ATFBaseDoorActor::IsDoorLocked() const
{
	return bRequiresKey && bIsLocked;
}

bool ATFBaseDoorActor::UnlockDoor(APawn* UnlockingCharacter)
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

	if (const ITFKeyHolderInterface* KeyHolder = Cast<ITFKeyHolderInterface>(Character))
	{
		return KeyHolder->HasKey(RequiredKeyID);
	}

	return false;
}

bool ATFBaseDoorActor::ToggleLock(APawn* Character)
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