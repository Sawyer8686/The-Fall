// Fill out your copyright notice in the Description page of Project Settings.

#include "TFPickupableActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"


ATFPickupableActor::ATFPickupableActor()
{
	PrimaryActorTick.bCanEverTick = false;
	InteractionDuration = 0.0f;
}

void ATFPickupableActor::BeginPlay()
{
	Super::BeginPlay();

	if (ItemData.ItemMesh && MeshComponent)
	{
		MeshComponent->SetStaticMesh(ItemData.ItemMesh);
	}
}

bool ATFPickupableActor::Interact(APawn* InstigatorPawn)
{
	bool bSuccess = OnPickup(InstigatorPawn);

	if (bSuccess)
	{
		OnItemPickedUp(InstigatorPawn);

		if (bDestroyOnPickup)
		{
			if (DestroyDelay > 0.0f)
			{
				if (MeshComponent)
				{
					MeshComponent->SetVisibility(false);
				}

				SetActorEnableCollision(false);

				SetLifeSpan(DestroyDelay);
			}
			else
			{
				Destroy();
			}
		}

		return true;
	}

	return false;
}

FInteractionData ATFPickupableActor::GetInteractionData(APawn* InstigatorPawn) const
{
	FInteractionData Data = Super::GetInteractionData(InstigatorPawn);
	return Data;
}

bool ATFPickupableActor::OnPickup(APawn* Picker)
{
	if (!CanPickup(Picker))
	{
		OnPickupFailed(Picker, FText::FromString("Cannot pickup item"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Picked up item: %s (x%d)"), *ItemData.ItemName.ToString(), ItemData.Quantity);

	return true;
}

FItemData ATFPickupableActor::GetItemData() const
{
	return ItemData;
}

bool ATFPickupableActor::CanPickup(APawn* Picker) const
{
	if (!Picker)
	{
		return false;
	}

	return true;
}

void ATFPickupableActor::OnPickupFailed(APawn* Picker, const FText& Reason)
{
	UE_LOG(LogTemp, Warning, TEXT("Pickup failed: %s"), *Reason.ToString());

	OnItemPickupFailed(Picker, Reason);
}

bool ATFPickupableActor::ShouldDestroyOnPickup() const
{
	return bDestroyOnPickup;
}

void ATFPickupableActor::SetItemData(const FItemData& NewItemData)
{
	ItemData = NewItemData;

	if (ItemData.ItemMesh && MeshComponent)
	{
		MeshComponent->SetStaticMesh(ItemData.ItemMesh);
	}
}

void ATFPickupableActor::SetQuantity(int32 NewQuantity)
{
	ItemData.Quantity = FMath::Max(1, NewQuantity);
}
