// Copyright TF Project. All Rights Reserved.

#include "TFPlayerCharacter.h"
#include "TFTypes.h"
#include "TFStaminaComponent.h"
#include "TFStatsComponent.h"
#include "TFInteractionComponent.h"
#include "TFInventoryComponent.h"
#include "TFPickupableActor.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
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

#pragma region Movement API

void ATFPlayerCharacter::StartSprinting()
{
	SetSprinting(true);
}

void ATFPlayerCharacter::StopSprinting()
{
	SetSprinting(false);
}

void ATFPlayerCharacter::StartSneaking()
{
	ATFCharacterBase::SetSneaking(true);
}

void ATFPlayerCharacter::StopSneaking()
{
	ATFCharacterBase::SetSneaking(false);
}

void ATFPlayerCharacter::TryJump()
{
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (CanCharacterJump() && MovementComp && !MovementComp->IsFalling())
	{
		HasJumped();
	}
}

#pragma endregion Movement API

#pragma region Sprint & Stamina

void ATFPlayerCharacter::SetSprinting(const bool bSprinting)
{
	if (bSprinting)
	{
		if (IsSneaking())
		{
			OnSprintBlocked(ESprintBlockReason::Sneaking);
			return;
		}

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

#pragma endregion Sprint & Stamina

#pragma region Stamina Events

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

#pragma endregion Stamina Events

#pragma region Jump

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

#pragma endregion Jump

#pragma region Inventory

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

	// Broadcast event for controller to handle
	OnBackpackEquipRequested.Broadcast(Slots, WeightLimit);

	return true;
}

void ATFPlayerCharacter::SetPendingBackpackActor(AActor* Actor)
{
	PendingBackpackActor = Actor;
}

void ATFPlayerCharacter::ConfirmBackpackEquip()
{
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
}

void ATFPlayerCharacter::CancelBackpackEquip()
{
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
		DroppedActor->SetItemData(DroppedItemData);

		if (UStaticMeshComponent* DroppedMesh = DroppedActor->GetMeshComponent())
		{
			if (!DroppedMesh->GetStaticMesh())
			{
				DroppedMesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")));
			}
		}
	}

	return true;
}

bool ATFPlayerCharacter::DropBackpack()
{
	if (!InventoryComponent || !InventoryComponent->HasBackpack())
	{
		return false;
	}

	int32 Slots = InventoryComponent->GetBackpackSlots();
	float WeightLimit = InventoryComponent->GetBackpackWeightLimit();

	TArray<FItemData> StoredItems = InventoryComponent->DeactivateBackpack();

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

		DroppedBackpack->SetItemData(BackpackData);

		if (UStaticMeshComponent* BackpackMeshComp = DroppedBackpack->GetMeshComponent())
		{
			if (!BackpackMeshComp->GetStaticMesh())
			{
				BackpackMeshComp->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
			}
		}
		DroppedBackpack->SetStoredInventoryItems(StoredItems);

		if (UStaticMeshComponent* BackpackMesh = DroppedBackpack->GetMeshComponent())
		{
			BackpackMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			DroppedBackpack->SetRootComponent(BackpackMesh);
			BackpackMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			BackpackMesh->SetCollisionResponseToAllChannels(ECR_Block);
			BackpackMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
			BackpackMesh->SetSimulatePhysics(true);

			FVector Impulse = GetActorForwardVector() * 200.0f + FVector(0.0f, 0.0f, 100.0f);
			BackpackMesh->AddImpulse(Impulse, NAME_None, true);
		}
	}

	UE_LOG(LogTFCharacter, Log, TEXT("Backpack dropped with %d items"), StoredItems.Num());
	return true;
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

#pragma endregion Inventory
