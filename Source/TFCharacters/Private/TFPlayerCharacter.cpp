// Copyright TF Project. All Rights Reserved.

#include "TFPlayerCharacter.h"
#include "TFTypes.h"
#include "TFStaminaComponent.h"
#include "TFStatsComponent.h"
#include "TFInteractionComponent.h"
#include "TFInventoryComponent.h"
#include "TFPickupableActor.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TFLockableInterface.h"
#include "GameFramework/CharacterMovementComponent.h"


ATFPlayerCharacter::ATFPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	StaminaComponent = CreateDefaultSubobject<UTFStaminaComponent>(TEXT("StaminaComponent"));
	StatsComponent = CreateDefaultSubobject<UTFStatsComponent>(TEXT("StatsComponent"));
	InteractionComponent = CreateDefaultSubobject<UTFInteractionComponent>(TEXT("InteractionComponent"));
	InventoryComponent = CreateDefaultSubobject<UTFInventoryComponent>(TEXT("InventoryComponent"));

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 150.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), "head");
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->SetRelativeLocation(FirstPersonCameraOffset);
	FirstPersonCamera->SetRelativeRotation(FirstPersonCameraRotation);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void ATFPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	BindStaminaEvents();
}

void ATFPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindStaminaEvents();

	Super::EndPlay(EndPlayReason);
}

void ATFPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext && !Subsystem->HasMappingContext(DefaultMappingContext))
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATFPlayerCharacter::Move);
		}

		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATFPlayerCharacter::Look);
		}

		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ATFPlayerCharacter::PlayerJump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}

		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ATFPlayerCharacter::SprintOn);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATFPlayerCharacter::SprintOff);
		}

		if (SneakAction)
		{
			EnhancedInputComponent->BindAction(SneakAction, ETriggerEvent::Started, this, &ATFPlayerCharacter::SneakOn);
			EnhancedInputComponent->BindAction(SneakAction, ETriggerEvent::Completed, this, &ATFPlayerCharacter::SneakOff);
		}

		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ATFPlayerCharacter::InteractPressed);
		}

		if (LockAction)
		{
			EnhancedInputComponent->BindAction(LockAction, ETriggerEvent::Started, this, &ATFPlayerCharacter::LockPressed);
			EnhancedInputComponent->BindAction(LockAction, ETriggerEvent::Completed, this, &ATFPlayerCharacter::LockReleased);
		}

		if (InventoryAction)
		{
			EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &ATFPlayerCharacter::InventoryPressed);
		}

		if (DropBackpackAction)
		{
			EnhancedInputComponent->BindAction(DropBackpackAction, ETriggerEvent::Started, this, &ATFPlayerCharacter::DropBackpackPressed);
		}
	}
}

void ATFPlayerCharacter::Move(const FInputActionValue& Value)
{
	if (bInventoryOpen || bConfirmDialogOpen)
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ATFPlayerCharacter::Look(const FInputActionValue& Value)
{
	if (bInventoryOpen || bConfirmDialogOpen)
	{
		return;
	}

	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(-LookAxisVector.Y);
	}
}

void ATFPlayerCharacter::SprintOn()
{
	if (bInventoryOpen || bConfirmDialogOpen) return;
	SetSprinting(true);
}

void ATFPlayerCharacter::SprintOff()
{
	if (bInventoryOpen || bConfirmDialogOpen) return;
	SetSprinting(false);
}

void ATFPlayerCharacter::SneakOn()
{
	if (bInventoryOpen || bConfirmDialogOpen) return;
	ATFCharacterBase::SetSneaking(true);
}

void ATFPlayerCharacter::SneakOff()
{
	if (bInventoryOpen || bConfirmDialogOpen) return;
	ATFCharacterBase::SetSneaking(false);
}

void ATFPlayerCharacter::PlayerJump()
{
	if (bInventoryOpen || bConfirmDialogOpen) return;
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (CanCharacterJump() && MovementComp && !MovementComp->IsFalling())
	{
		HasJumped();
	}
}

void ATFPlayerCharacter::InteractPressed()
{
	if (bInventoryOpen || bConfirmDialogOpen) return;
	if (InteractionComponent)
	{
		InteractionComponent->Interact();
	}
}

void ATFPlayerCharacter::SetSprinting(const bool bSprinting)
{
	if (bSprinting)
	{
		// Cannot sprint while sneaking
		if (IsSneaking())
		{
			OnSprintBlocked(ESprintBlockReason::Sneaking);
			return;
		}

		// Cannot sprint without stamina component or when exhausted
		if (!StaminaComponent || !StaminaComponent->CanSprint())
		{
			OnSprintBlocked(ESprintBlockReason::NoStamina);
			return;
		}

		bIsSprinting = true;
		StaminaComponent->StartStaminaDrain(StaminaComponent->SprintDrainRate);
	}
	else
	{
		if (!bIsSprinting)
		{
			return;
		}

		bIsSprinting = false;

		if (StaminaComponent)
		{
			StaminaComponent->StopStaminaDrain();
		}
	}
	UpdateMovementSpeed();
}

float ATFPlayerCharacter::GetSprintSpeed() const
{
	return SprintSpeed;
}

void ATFPlayerCharacter::UpdateMovementSpeed()
{
	if (!GetCharacterMovement())
	{
		return;
	}

	float TargetSpeed = GetWalkSpeed();

	if (bIsSprinting)
	{
		TargetSpeed = SprintSpeed;
	}
	else if (IsSneaking())
	{
		TargetSpeed = GetSneakSpeed();
	}

	if (StaminaComponent && StaminaComponent->IsExhausted())
	{
		TargetSpeed *= ExhaustedSpeedMultiplier;
	}

	GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
}

void ATFPlayerCharacter::BindStaminaEvents()
{
	if (StaminaComponent)
	{
		StaminaComponent->OnStaminaDepleted.AddUObject(this, &ATFPlayerCharacter::HandleStaminaDepleted);
		StaminaComponent->OnStaminaRecovered.AddUObject(this, &ATFPlayerCharacter::HandleStaminaRecovered);
	}
}

void ATFPlayerCharacter::UnbindStaminaEvents()
{
	if (StaminaComponent)
	{
		StaminaComponent->OnStaminaDepleted.RemoveAll(this);
		StaminaComponent->OnStaminaRecovered.RemoveAll(this);
	}
}

void ATFPlayerCharacter::HandleStaminaDepleted()
{
	if (bIsSprinting)
	{
		SetSprinting(false);
	}

	UpdateMovementSpeed();

	OnStaminaDepleted();
}

void ATFPlayerCharacter::HandleStaminaRecovered()
{
	UpdateMovementSpeed();

	OnStaminaRecovered();
}

void ATFPlayerCharacter::OnStaminaDepleted()
{
}

void ATFPlayerCharacter::OnStaminaRecovered()
{
}

bool ATFPlayerCharacter::CanCharacterJump() const
{
	if (!ATFCharacterBase::CanCharacterJump())
	{
		return false;
	}

	if (StaminaComponent)
	{
		return StaminaComponent->CanJump();
	}

	return true;
}

void ATFPlayerCharacter::HasJumped()
{
	if (StaminaComponent)
	{
		StaminaComponent->ConsumeStamina(
			StaminaComponent->JumpStaminaCost,
			EStaminaDrainReason::Jump
		);
	}
	ATFCharacterBase::HasJumped();
}

bool ATFPlayerCharacter::HasKey(FName KeyID) const
{
	return CollectedKeys.Contains(KeyID);
}

void ATFPlayerCharacter::AddKey(FName KeyID, const FText& KeyName)
{
	if (KeyID.IsNone())
	{
		return;
	}

	if (CollectedKeys.Contains(KeyID))
	{
		return;
	}

	FText DisplayName = KeyName.IsEmpty() ? FText::FromName(KeyID) : KeyName;
	CollectedKeys.Add(KeyID, DisplayName);

	OnKeyAdded(KeyID);
	OnKeyCollectionChanged.Broadcast();
	UE_LOG(LogTFCharacter, Log, TEXT("Key added: %s (%s)"), *KeyID.ToString(), *DisplayName.ToString());
}

bool ATFPlayerCharacter::RemoveKey(FName KeyID)
{
	if (KeyID.IsNone())
	{
		return false;
	}

	int32 NumRemoved = CollectedKeys.Remove(KeyID);

	if (NumRemoved > 0)
	{
		OnKeyRemoved(KeyID);
		OnKeyCollectionChanged.Broadcast();
		UE_LOG(LogTFCharacter, Log, TEXT("Key removed: %s"), *KeyID.ToString());
		return true;
	}

	return false;
}

void ATFPlayerCharacter::LockPressed()
{
	if (bInventoryOpen || bConfirmDialogOpen) return;

	if (!InteractionComponent)
	{
		return;
	}

	AActor* Target = InteractionComponent->GetCurrentInteractable();
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

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			LockHoldTimerHandle,
			this,
			&ATFPlayerCharacter::CompleteLockAction,
			Duration,
			false
		);
	}
}

void ATFPlayerCharacter::LockReleased()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LockHoldTimerHandle);
	}
	LockTarget = nullptr;
}

void ATFPlayerCharacter::CompleteLockAction()
{
	if (!LockTarget.IsValid())
	{
		return;
	}

	if (ITFLockableInterface* Lockable = Cast<ITFLockableInterface>(LockTarget.Get()))
	{
		Lockable->ToggleLock(this);
	}

	LockTarget = nullptr;
}

void ATFPlayerCharacter::SetUIInputMode(bool bShowCursor)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	if (bShowCursor)
	{
		PC->bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
	}
	else
	{
		PC->bShowMouseCursor = false;
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->ResetIgnoreMoveInput();
		PC->ResetIgnoreLookInput();
	}
}

void ATFPlayerCharacter::InventoryPressed()
{
	if (bConfirmDialogOpen)
	{
		return;
	}

	if (!InventoryComponent || !InventoryComponent->HasBackpack())
	{
		return;
	}

	bInventoryOpen = !bInventoryOpen;

	if (bInventoryOpen && bIsSprinting)
	{
		SetSprinting(false);
	}

	SetUIInputMode(bInventoryOpen);

	OnInventoryToggled.Broadcast(bInventoryOpen);
}

bool ATFPlayerCharacter::DropItem(FName ItemID)
{
	if (!InventoryComponent || ItemID.IsNone())
	{
		return false;
	}

	const FItemData* ItemPtr = InventoryComponent->GetItem(ItemID);
	if (!ItemPtr)
	{
		return false;
	}

	FItemData DroppedItemData = *ItemPtr;

	if (!InventoryComponent->RemoveItem(ItemID))
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return true;
	}

	FVector DropLocation = GetActorLocation() + GetActorForwardVector() * 150.0f;
	DropLocation.Z -= 50.0f;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ATFPickupableActor* DroppedActor = World->SpawnActor<ATFPickupableActor>(
		ATFPickupableActor::StaticClass(),
		DropLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (DroppedActor)
	{
		if (!DroppedItemData.ItemMesh)
		{
			DroppedItemData.ItemMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
		}
		DroppedActor->SetItemData(DroppedItemData);
	}

	return true;
}

void ATFPlayerCharacter::DropBackpackPressed()
{
	if (bConfirmDialogOpen) return;

	DropBackpack();
}

bool ATFPlayerCharacter::DropBackpack()
{
	if (!InventoryComponent || !InventoryComponent->HasBackpack())
	{
		return false;
	}

	// Close inventory if open
	if (bInventoryOpen)
	{
		InventoryPressed();
	}

	// Get backpack info before deactivating
	int32 Slots = InventoryComponent->GetBackpackSlots();
	float WeightLimit = InventoryComponent->GetBackpackWeightLimit();

	// Deactivate backpack and get all items
	TArray<FItemData> StoredItems = InventoryComponent->DeactivateBackpack();

	// Spawn backpack actor in front of player
	UWorld* World = GetWorld();
	if (!World)
	{
		return true;
	}

	FVector DropLocation = GetActorLocation() + GetActorForwardVector() * 100.0f;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ATFPickupableActor* DroppedBackpack = World->SpawnActor<ATFPickupableActor>(
		ATFPickupableActor::StaticClass(),
		DropLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (DroppedBackpack)
	{
		FItemData BackpackData = EquippedBackpackData;
		BackpackData.BackpackSlots = Slots;
		BackpackData.BackpackWeightLimit = WeightLimit;

		if (!BackpackData.ItemMesh)
		{
			BackpackData.ItemMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		}

		DroppedBackpack->SetItemData(BackpackData);
		DroppedBackpack->SetStoredInventoryItems(StoredItems);

		// Enable physics on the dropped backpack mesh
		if (UStaticMeshComponent* BackpackMesh = DroppedBackpack->GetMeshComponent())
		{
			BackpackMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			DroppedBackpack->SetRootComponent(BackpackMesh);
			BackpackMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			BackpackMesh->SetCollisionResponseToAllChannels(ECR_Block);
			BackpackMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
			BackpackMesh->SetSimulatePhysics(true);

			// Add forward impulse to toss the backpack slightly ahead
			FVector Impulse = GetActorForwardVector() * 200.0f + FVector(0.0f, 0.0f, 100.0f);
			BackpackMesh->AddImpulse(Impulse, NAME_None, true);
		}
	}

	UE_LOG(LogTFCharacter, Log, TEXT("Backpack dropped with %d items"), StoredItems.Num());
	return true;
}

bool ATFPlayerCharacter::HasBackpack() const
{
	return InventoryComponent && InventoryComponent->HasBackpack();
}

bool ATFPlayerCharacter::ActivateBackpack(int32 Slots, float WeightLimit)
{
	if (!InventoryComponent || InventoryComponent->HasBackpack())
	{
		return false;
	}

	PendingBackpackSlots = Slots;
	PendingBackpackWeightLimit = WeightLimit;
	bConfirmDialogOpen = true;

	if (bIsSprinting)
	{
		SetSprinting(false);
	}

	SetUIInputMode(true);

	OnBackpackEquipRequested.Broadcast(Slots, WeightLimit);
	return true;
}

void ATFPlayerCharacter::SetPendingBackpackActor(AActor* Actor)
{
	PendingBackpackActor = Actor;
}

void ATFPlayerCharacter::ConfirmBackpackEquip()
{
	bConfirmDialogOpen = false;

	TArray<FItemData> ItemsToRestore;

	if (PendingBackpackActor.IsValid())
	{
		if (ATFPickupableActor* BackpackActor = Cast<ATFPickupableActor>(PendingBackpackActor.Get()))
		{
			EquippedBackpackData = BackpackActor->GetItemData();
			ItemsToRestore = BackpackActor->GetStoredInventoryItems();
		}
	}

	if (InventoryComponent)
	{
		InventoryComponent->ActivateBackpack(PendingBackpackSlots, PendingBackpackWeightLimit);

		if (ItemsToRestore.Num() > 0)
		{
			InventoryComponent->RestoreItems(ItemsToRestore);
		}
	}

	if (PendingBackpackActor.IsValid())
	{
		PendingBackpackActor->Destroy();
		PendingBackpackActor = nullptr;
	}

	SetUIInputMode(false);
}

void ATFPlayerCharacter::CancelBackpackEquip()
{
	bConfirmDialogOpen = false;

	if (PendingBackpackActor.IsValid())
	{
		PendingBackpackActor->SetActorEnableCollision(true);

		if (ATFPickupableActor* BackpackActor = Cast<ATFPickupableActor>(PendingBackpackActor.Get()))
		{
			if (UStaticMeshComponent* BackpackMesh = BackpackActor->GetMeshComponent())
			{
				BackpackMesh->SetSimulatePhysics(true);
			}
		}

		PendingBackpackActor = nullptr;
	}

	SetUIInputMode(false);
}

bool ATFPlayerCharacter::AddItem(const FItemData& Item)
{
	if (!InventoryComponent)
	{
		return false;
	}
	return InventoryComponent->AddItem(Item);
}

bool ATFPlayerCharacter::RemoveItem(FName ItemID)
{
	if (!InventoryComponent)
	{
		return false;
	}
	return InventoryComponent->RemoveItem(ItemID);
}

bool ATFPlayerCharacter::HasItem(FName ItemID) const
{
	return InventoryComponent && InventoryComponent->HasItem(ItemID);
}

bool ATFPlayerCharacter::HasSpaceForItem(const FItemData& Item) const
{
	return InventoryComponent && InventoryComponent->HasSpaceForItem(Item);
}

bool ATFPlayerCharacter::CanCarryWeight(float AdditionalWeight) const
{
	return InventoryComponent && InventoryComponent->CanCarryWeight(AdditionalWeight);
}

int32 ATFPlayerCharacter::GetFreeSlots() const
{
	return InventoryComponent ? InventoryComponent->GetFreeSlots() : 0;
}

float ATFPlayerCharacter::GetRemainingCapacity() const
{
	return InventoryComponent ? InventoryComponent->GetRemainingCapacity() : 0.0f;
}