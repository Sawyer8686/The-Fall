// Copyright TF Project. All Rights Reserved.

#include "TFPlayerController.h"
#include "TFLockProgressManager.h"
#include "TFPlayerCharacter.h"
#include "TFInteractionComponent.h"
#include "TFInventoryComponent.h"
#include "TFLockableInterface.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ATFPlayerController::ATFPlayerController()
{
	LockProgressManager = CreateDefaultSubobject<UTFLockProgressManager>(TEXT("LockProgressManager"));
}

void ATFPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Add input mapping context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext && !Subsystem->HasMappingContext(DefaultMappingContext))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ATFPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ATFPlayerCharacter* NewCharacter = Cast<ATFPlayerCharacter>(InPawn);
	CachedPlayerCharacter = NewCharacter;

	if (NewCharacter)
	{
		BindToCharacter(NewCharacter);
	}
}

void ATFPlayerController::OnUnPossess()
{
	if (CachedPlayerCharacter.IsValid())
	{
		UnbindFromCharacter(CachedPlayerCharacter.Get());
	}

	CachedPlayerCharacter = nullptr;

	Super::OnUnPossess();
}

void ATFPlayerController::BindToCharacter(ATFPlayerCharacter* InCharacter)
{
	if (!InCharacter)
	{
		return;
	}

	InCharacter->OnBackpackEquipRequested.AddUObject(this, &ATFPlayerController::HandleBackpackEquipRequested);
}

void ATFPlayerController::UnbindFromCharacter(ATFPlayerCharacter* InCharacter)
{
	if (!InCharacter)
	{
		return;
	}

	InCharacter->OnBackpackEquipRequested.RemoveAll(this);
}

void ATFPlayerController::HandleBackpackEquipRequested(int32 Slots, float WeightLimit)
{
	OpenBackpackConfirmDialog(Slots, WeightLimit);
}

void ATFPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATFPlayerController::HandleMove);
		}

		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATFPlayerController::HandleLook);
		}

		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ATFPlayerController::HandleJumpStarted);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ATFPlayerController::HandleJumpCompleted);
		}

		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ATFPlayerController::HandleSprintStarted);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATFPlayerController::HandleSprintCompleted);
		}

		if (SneakAction)
		{
			EnhancedInputComponent->BindAction(SneakAction, ETriggerEvent::Started, this, &ATFPlayerController::HandleSneakStarted);
			EnhancedInputComponent->BindAction(SneakAction, ETriggerEvent::Completed, this, &ATFPlayerController::HandleSneakCompleted);
		}

		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ATFPlayerController::HandleInteract);
		}

		if (LockAction)
		{
			EnhancedInputComponent->BindAction(LockAction, ETriggerEvent::Started, this, &ATFPlayerController::HandleLockStarted);
			EnhancedInputComponent->BindAction(LockAction, ETriggerEvent::Completed, this, &ATFPlayerController::HandleLockCompleted);
		}

		if (InventoryAction)
		{
			EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &ATFPlayerController::HandleInventory);
		}

		if (DropBackpackAction)
		{
			EnhancedInputComponent->BindAction(DropBackpackAction, ETriggerEvent::Started, this, &ATFPlayerController::HandleDropBackpack);
		}
	}
}

ATFPlayerCharacter* ATFPlayerController::GetTFPlayerCharacter() const
{
	if (CachedPlayerCharacter.IsValid())
	{
		return CachedPlayerCharacter.Get();
	}
	return Cast<ATFPlayerCharacter>(GetPawn());
}

void ATFPlayerController::SetUIInputMode(bool bShowCursor)
{
	if (bShowCursor)
	{
		bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		SetIgnoreMoveInput(true);
		SetIgnoreLookInput(true);
	}
	else
	{
		bShowMouseCursor = false;
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		ResetIgnoreMoveInput();
		ResetIgnoreLookInput();
	}
}

void ATFPlayerController::ToggleInventory()
{
	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (!PlayerChar)
	{
		return;
	}

	UTFInventoryComponent* InventoryComp = PlayerChar->GetInventoryComponent();
	if (!InventoryComp || !InventoryComp->HasBackpack())
	{
		return;
	}

	if (bConfirmDialogOpen)
	{
		return;
	}

	bInventoryOpen = !bInventoryOpen;

	if (bInventoryOpen && PlayerChar->IsSprinting())
	{
		PlayerChar->StopSprinting();
	}

	SetUIInputMode(bInventoryOpen);
	OnInventoryToggled.Broadcast(bInventoryOpen);
}

void ATFPlayerController::OpenBackpackConfirmDialog(int32 Slots, float WeightLimit)
{
	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (!PlayerChar)
	{
		return;
	}

	bConfirmDialogOpen = true;

	if (PlayerChar->IsSprinting())
	{
		PlayerChar->StopSprinting();
	}

	SetUIInputMode(true);
	OnBackpackEquipRequested.Broadcast(Slots, WeightLimit);
}

void ATFPlayerController::CloseBackpackConfirmDialog(bool bConfirmed)
{
	bConfirmDialogOpen = false;

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (PlayerChar)
	{
		if (bConfirmed)
		{
			PlayerChar->ConfirmBackpackEquip();
		}
		else
		{
			PlayerChar->CancelBackpackEquip();
		}
	}

	SetUIInputMode(false);
}

#pragma region Input Handlers

void ATFPlayerController::HandleMove(const FInputActionValue& Value)
{
	if (IsUIBlockingInput())
	{
		return;
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (!PlayerChar)
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	PlayerChar->AddMovementInput(ForwardDirection, MovementVector.Y);
	PlayerChar->AddMovementInput(RightDirection, MovementVector.X);
}

void ATFPlayerController::HandleLook(const FInputActionValue& Value)
{
	if (IsUIBlockingInput())
	{
		return;
	}

	FVector2D LookAxisVector = Value.Get<FVector2D>();
	AddYawInput(LookAxisVector.X);
	AddPitchInput(-LookAxisVector.Y);
}

void ATFPlayerController::HandleJumpStarted()
{
	if (IsUIBlockingInput())
	{
		return;
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (PlayerChar)
	{
		PlayerChar->TryJump();
	}
}

void ATFPlayerController::HandleJumpCompleted()
{
	if (IsUIBlockingInput())
	{
		return;
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (PlayerChar)
	{
		PlayerChar->StopJumping();
	}
}

void ATFPlayerController::HandleSprintStarted()
{
	if (IsUIBlockingInput())
	{
		return;
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (PlayerChar)
	{
		PlayerChar->StartSprinting();
	}
}

void ATFPlayerController::HandleSprintCompleted()
{
	if (IsUIBlockingInput())
	{
		return;
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (PlayerChar)
	{
		PlayerChar->StopSprinting();
	}
}

void ATFPlayerController::HandleSneakStarted()
{
	if (IsUIBlockingInput())
	{
		return;
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (PlayerChar)
	{
		PlayerChar->StartSneaking();
	}
}

void ATFPlayerController::HandleSneakCompleted()
{
	if (IsUIBlockingInput())
	{
		return;
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (PlayerChar)
	{
		PlayerChar->StopSneaking();
	}
}

void ATFPlayerController::HandleInteract()
{
	if (IsUIBlockingInput())
	{
		return;
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (PlayerChar)
	{
		UTFInteractionComponent* InteractionComp = PlayerChar->GetInteractionComponent();
		if (InteractionComp)
		{
			InteractionComp->Interact();
		}
	}
}

void ATFPlayerController::HandleLockStarted()
{
	if (IsUIBlockingInput())
	{
		return;
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (!PlayerChar)
	{
		return;
	}

	UTFInteractionComponent* InteractionComp = PlayerChar->GetInteractionComponent();
	if (!InteractionComp)
	{
		return;
	}

	AActor* Target = InteractionComp->GetCurrentInteractable();
	if (!Target)
	{
		return;
	}

	ITFLockableInterface* Lockable = Cast<ITFLockableInterface>(Target);
	if (!Lockable)
	{
		return;
	}

	float Duration = Lockable->GetLockDuration();
	LockTarget = Target;

	if (Duration <= 0.0f)
	{
		CompleteLockAction();
		return;
	}

	bIsUnlockingAction = Lockable->IsCurrentlyLocked();
	LockActionDuration = Duration;
	LockActionElapsedTime = 0.0f;

	OnLockActionStarted.Broadcast(Duration, bIsUnlockingAction);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			LockHoldTimerHandle,
			this,
			&ATFPlayerController::CompleteLockAction,
			Duration,
			false
		);

		World->GetTimerManager().SetTimer(
			LockProgressTimerHandle,
			this,
			&ATFPlayerController::UpdateLockProgress,
			0.016f,
			true
		);
	}
}

void ATFPlayerController::HandleLockCompleted()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LockHoldTimerHandle);
		World->GetTimerManager().ClearTimer(LockProgressTimerHandle);
	}

	if (LockTarget.IsValid() && LockActionDuration > 0.0f)
	{
		OnLockActionCancelled.Broadcast();
	}

	LockTarget = nullptr;
	LockActionDuration = 0.0f;
	LockActionElapsedTime = 0.0f;
}

void ATFPlayerController::HandleInventory()
{
	ToggleInventory();
}

void ATFPlayerController::HandleDropBackpack()
{
	if (bConfirmDialogOpen)
	{
		return;
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (PlayerChar)
	{
		// Close inventory if open before dropping
		if (bInventoryOpen)
		{
			ToggleInventory();
		}
		PlayerChar->DropBackpack();
	}
}

#pragma endregion Input Handlers

#pragma region Lock Action

void ATFPlayerController::UpdateLockProgress()
{
	if (!LockTarget.IsValid() || LockActionDuration <= 0.0f)
	{
		return;
	}

	LockActionElapsedTime += 0.016f;
	LockActionElapsedTime = FMath::Min(LockActionElapsedTime, LockActionDuration);

	OnLockActionProgress.Broadcast(LockActionElapsedTime);
}

void ATFPlayerController::CompleteLockAction()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LockProgressTimerHandle);
	}

	if (!LockTarget.IsValid())
	{
		return;
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (ITFLockableInterface* Lockable = Cast<ITFLockableInterface>(LockTarget.Get()))
	{
		Lockable->ToggleLock(PlayerChar);
	}

	OnLockActionCompleted.Broadcast();

	LockTarget = nullptr;
	LockActionDuration = 0.0f;
	LockActionElapsedTime = 0.0f;
}

#pragma endregion Lock Action
