// Copyright TF Project. All Rights Reserved.

#include "TFBaseDoorActor.h"
#include "TFTypes.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Misc/ConfigCacheIni.h"

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

	OneShotAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("OneShotAudio"));
	OneShotAudioComponent->SetupAttachment(DoorMesh);
	OneShotAudioComponent->bAutoActivate = false;

	LoopAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("LoopAudio"));
	LoopAudioComponent->SetupAttachment(DoorMesh);
	LoopAudioComponent->bAutoActivate = false;

	MaxInteractionDistance = 200.0f;

	//bIsReusable = true;
}

void ATFBaseDoorActor::BeginPlay()
{
	// Use DoorID as InteractableID if not explicitly set
	if (InteractableID.IsNone() && !DoorID.IsNone())
	{
		InteractableID = DoorID;
	}

	// Call parent BeginPlay which handles base data-driven config
	Super::BeginPlay();

	InitialRotation = DoorMesh->GetRelativeRotation();
}

void ATFBaseDoorActor::LoadConfigFromINI()
{
	Super::LoadConfigFromINI();

	if (DoorID.IsNone())
	{
		return;
	}

	const FString SectionName = DoorID.ToString();
	FString ConfigFilePath;

	if (!TFConfigUtils::LoadINISection(TEXT("DoorConfig.ini"), SectionName, ConfigFilePath, LogTFDoor))
	{
		return;
	}

	UE_LOG(LogTFDoor, Log, TEXT("ATFBaseDoorActor: Loading config for DoorID '%s'"), *SectionName);

#pragma region Door Settings

	FString StringValue;
	if (GConfig->GetString(*SectionName, TEXT("HingeType"), StringValue, ConfigFilePath))
	{
		static const TMap<FString, EDoorHinge> HingeMap = {
			{TEXT("Left"), EDoorHinge::Left},
			{TEXT("Right"), EDoorHinge::Right}
		};

		bool bMatched = false;
		HingeType = TFConfigUtils::StringToEnum(StringValue, HingeMap, EDoorHinge::Left, &bMatched);
		if (!bMatched)
		{
			UE_LOG(LogTFDoor, Warning, TEXT("ATFBaseDoorActor: Unknown HingeType '%s', defaulting to Left"), *StringValue);
		}
	}

	GConfig->GetFloat(*SectionName, TEXT("MaxOpenAngle"), MaxOpenAngle, ConfigFilePath);
	GConfig->GetFloat(*SectionName, TEXT("OpenDuration"), OpenDuration, ConfigFilePath);
	GConfig->GetFloat(*SectionName, TEXT("CloseDuration"), CloseDuration, ConfigFilePath);
	GConfig->GetBool(*SectionName, TEXT("bAutoClose"), bAutoClose, ConfigFilePath);
	GConfig->GetFloat(*SectionName, TEXT("AutoCloseDelay"), AutoCloseDelay, ConfigFilePath);
	GConfig->GetBool(*SectionName, TEXT("bCanOpenFromBothSides"), bCanOpenFromBothSides, ConfigFilePath);

	MaxOpenAngle = FMath::Clamp(MaxOpenAngle, 0.0f, 180.0f);
	OpenDuration = FMath::Clamp(OpenDuration, 0.1f, 5.0f);
	CloseDuration = FMath::Clamp(CloseDuration, 0.1f, 5.0f);
	AutoCloseDelay = FMath::Max(AutoCloseDelay, 0.0f);

#pragma endregion Door Settings

#pragma region Key Settings

	GConfig->GetBool(*SectionName, TEXT("bRequiresKey"), bRequiresKey, ConfigFilePath);

	if (GConfig->GetString(*SectionName, TEXT("RequiredKeyID"), StringValue, ConfigFilePath) && !StringValue.IsEmpty())
	{
		RequiredKeyID = FName(*StringValue);
	}

	if (GConfig->GetString(*SectionName, TEXT("RequiredKeyName"), StringValue, ConfigFilePath))
	{
		RequiredKeyName = FText::FromString(StringValue);
	}

	GConfig->GetBool(*SectionName, TEXT("bIsLocked"), bIsLocked, ConfigFilePath);
	GConfig->GetBool(*SectionName, TEXT("bCanRelock"), bCanRelock, ConfigFilePath);

#pragma endregion Key Settings

#pragma region Mesh Loading

	if (UStaticMesh* LoadedMesh = TFConfigUtils::LoadAssetFromConfig<UStaticMesh>(SectionName, TEXT("DoorFrameMesh"), ConfigFilePath, LogTFDoor, TEXT("DoorFrameMesh")))
	{
		DoorFrameMesh->SetStaticMesh(LoadedMesh);
	}

	if (UStaticMesh* LoadedMesh = TFConfigUtils::LoadAssetFromConfig<UStaticMesh>(SectionName, TEXT("DoorMesh"), ConfigFilePath, LogTFDoor, TEXT("DoorMesh")))
	{
		DoorMesh->SetStaticMesh(LoadedMesh);
	}

#pragma endregion Mesh Loading

#pragma region Audio Loading

	DoorOpenSound = TFConfigUtils::LoadAssetFromConfig<USoundBase>(SectionName, TEXT("DoorOpenSound"), ConfigFilePath, LogTFDoor, TEXT("DoorOpenSound"));
	DoorCloseSound = TFConfigUtils::LoadAssetFromConfig<USoundBase>(SectionName, TEXT("DoorCloseSound"), ConfigFilePath, LogTFDoor, TEXT("DoorCloseSound"));
	DoorLockedSound = TFConfigUtils::LoadAssetFromConfig<USoundBase>(SectionName, TEXT("DoorLockedSound"), ConfigFilePath, LogTFDoor, TEXT("DoorLockedSound"));
	DoorMovementSound = TFConfigUtils::LoadAssetFromConfig<USoundBase>(SectionName, TEXT("DoorMovementSound"), ConfigFilePath, LogTFDoor, TEXT("DoorMovementSound"));
	DoorUnlockSound = TFConfigUtils::LoadAssetFromConfig<USoundBase>(SectionName, TEXT("DoorUnlockSound"), ConfigFilePath, LogTFDoor, TEXT("DoorUnlockSound"));
	DoorLockSound = TFConfigUtils::LoadAssetFromConfig<USoundBase>(SectionName, TEXT("DoorLockSound"), ConfigFilePath, LogTFDoor, TEXT("DoorLockSound"));

#pragma endregion Audio Loading

	UE_LOG(LogTFDoor, Log, TEXT("ATFBaseDoorActor: Config loaded successfully for DoorID '%s'"), *SectionName);
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

	// Open away from the player: positive angle if player is in front, negative if behind
	return (DotProduct >= 0.0f) ? MaxOpenAngle : -MaxOpenAngle;
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

	if (bAutoClose)
	{
		if (AutoCloseDelay <= 0.0f)
		{
			AutoCloseDoor();
		}
		else if (UWorld* World = GetWorld())
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
	if (!Sound || !OneShotAudioComponent)
	{
		return;
	}

	OneShotAudioComponent->SetSound(Sound);
	OneShotAudioComponent->Play();
}

void ATFBaseDoorActor::PlayDoorMovementSound()
{
	if (!DoorMovementSound || !LoopAudioComponent)
	{
		return;
	}

	if (!LoopAudioComponent->IsPlaying())
	{
		LoopAudioComponent->SetSound(DoorMovementSound);
		LoopAudioComponent->Play();
	}
}

void ATFBaseDoorActor::StopDoorMovementSound()
{
	if (LoopAudioComponent && LoopAudioComponent->IsPlaying())
	{
		LoopAudioComponent->Stop();
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

	if (IsMoving())
	{
		Data.bCanInteract = false;
		return Data;
	}

	if (bRequiresKey && bIsLocked)
	{
		Data.bCanInteract = CharacterHasKey(InstigatorPawn);
		return Data;
	}

	Data.bCanInteract = IsClosed() || IsOpen();
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

	UE_LOG(LogTFDoor, Log, TEXT("ATFBaseDoorActor: Door unlocked with key '%s'"), *RequiredKeyID.ToString());

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

	UE_LOG(LogTFDoor, Log, TEXT("ATFBaseDoorActor: Door locked with key '%s'"), *RequiredKeyID.ToString());

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