// Fill out your copyright notice in the Description page of Project Settings.

#include "TFPickupableActor.h"
#include "TFPlayerCharacter.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

ATFPickupableActor::ATFPickupableActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create rotating movement component
	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	RotatingMovement->RotationRate = FRotator(0.0f, RotationSpeed, 0.0f);

	// Set default interaction text for pickups
	InteractionText = FText::FromString("Pick Up");

	// Pickups are instant by default
	InteractionDuration = 0.0f;
}

void ATFPickupableActor::BeginPlay()
{
	Super::BeginPlay();

	// Store initial Z location for bobbing
	InitialZ = GetActorLocation().Z;

	// Configure rotating movement
	if (RotatingMovement)
	{
		RotatingMovement->SetActive(bEnableRotation);
		RotatingMovement->RotationRate = FRotator(0.0f, RotationSpeed, 0.0f);
	}

	// Set item mesh if specified
	if (ItemData.ItemMesh && MeshComponent)
	{
		MeshComponent->SetStaticMesh(ItemData.ItemMesh);
	}
}

void ATFPickupableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update bobbing animation
	if (bEnableBobbing)
	{
		UpdateBobbing(DeltaTime);
	}
}

void ATFPickupableActor::UpdateBobbing(float DeltaTime)
{
	BobbingTimer += DeltaTime * BobbingFrequency;

	// Calculate bobbing offset
	float BobbingOffset = FMath::Sin(BobbingTimer * 2.0f * PI) * BobbingAmplitude;

	// Apply bobbing to Z location
	FVector CurrentLocation = GetActorLocation();
	CurrentLocation.Z = InitialZ + BobbingOffset;
	SetActorLocation(CurrentLocation);
}

void ATFPickupableActor::PlayPickupEffects()
{
	if (!bPlayPickupEffects)
	{
		return;
	}

	// Play sound
	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
	}

	// Spawn particle effect
	if (PickupParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PickupParticle, GetActorLocation());
	}
}

bool ATFPickupableActor::Interact_Implementation(ATFPlayerCharacter* InstigatorCharacter)
{
	// Attempt pickup
	bool bSuccess = OnPickup_Implementation(InstigatorCharacter);

	if (bSuccess)
	{
		// Play effects
		PlayPickupEffects();

		// Call blueprint event
		OnItemPickedUp(InstigatorCharacter);

		// Destroy if needed
		if (bDestroyOnPickup)
		{
			if (DestroyDelay > 0.0f)
			{
				// Hide mesh immediately
				if (MeshComponent)
				{
					MeshComponent->SetVisibility(false);
				}

				// Disable collision
				SetActorEnableCollision(false);

				// Destroy after delay
				SetLifeSpan(DestroyDelay);
			}
			else
			{
				// Destroy immediately
				Destroy();
			}
		}

		return true;
	}

	return false;
}

FInteractionData ATFPickupableActor::GetInteractionData_Implementation(ATFPlayerCharacter* InstigatorCharacter) const
{
	FInteractionData Data = Super::GetInteractionData_Implementation(InstigatorCharacter);

	// Override with item-specific text
	if (!ItemData.ItemName.IsEmpty())
	{
		Data.InteractionText = FText::Format(
			FText::FromString("Pick Up {0}"),
			ItemData.ItemName
		);
	}

	// Show quantity if stackable and > 1
	if (ItemData.bIsStackable && ItemData.Quantity > 1)
	{
		Data.SecondaryText = FText::Format(
			FText::FromString("x{0}"),
			FText::AsNumber(ItemData.Quantity)
		);
	}

	// Use item icon
	if (ItemData.ItemIcon)
	{
		Data.InteractionIcon = ItemData.ItemIcon;
	}

	return Data;
}

bool ATFPickupableActor::OnPickup_Implementation(ATFPlayerCharacter* PickerCharacter)
{
	if (!CanPickup_Implementation(PickerCharacter))
	{
		OnPickupFailed_Implementation(PickerCharacter, FText::FromString("Cannot pickup item"));
		return false;
	}

	// TODO: Add to inventory when inventory system is implemented
	// For now, just return true
	// PickerCharacter->GetInventory()->AddItem(ItemData);

	UE_LOG(LogTemp, Log, TEXT("Picked up item: %s (x%d)"), *ItemData.ItemName.ToString(), ItemData.Quantity);

	return true;
}

FItemData ATFPickupableActor::GetItemData_Implementation() const
{
	return ItemData;
}

bool ATFPickupableActor::CanPickup_Implementation(ATFPlayerCharacter* PickerCharacter) const
{
	if (!PickerCharacter)
	{
		return false;
	}

	// TODO: Check inventory space when inventory system is implemented
	// if (!PickerCharacter->GetInventory()->HasSpace(ItemData))
	// {
	//     return false;
	// }

	return true;
}

void ATFPickupableActor::OnPickupFailed_Implementation(ATFPlayerCharacter* PickerCharacter, const FText& Reason)
{
	// Log failure
	UE_LOG(LogTemp, Warning, TEXT("Pickup failed: %s"), *Reason.ToString());

	// Call blueprint event
	OnItemPickupFailed(PickerCharacter, Reason);

	// TODO: Show UI message when UI system is ready
}

bool ATFPickupableActor::ShouldDestroyOnPickup_Implementation() const
{
	return bDestroyOnPickup;
}

void ATFPickupableActor::SetItemData(const FItemData& NewItemData)
{
	ItemData = NewItemData;

	// Update mesh if changed
	if (ItemData.ItemMesh && MeshComponent)
	{
		MeshComponent->SetStaticMesh(ItemData.ItemMesh);
	}

	// Update interaction text
	if (!ItemData.ItemName.IsEmpty())
	{
		InteractionText = FText::Format(
			FText::FromString("Pick Up {0}"),
			ItemData.ItemName
		);
	}
}

void ATFPickupableActor::SetQuantity(int32 NewQuantity)
{
	ItemData.Quantity = FMath::Max(1, NewQuantity);
}
