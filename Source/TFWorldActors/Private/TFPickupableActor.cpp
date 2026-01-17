// Fill out your copyright notice in the Description page of Project Settings.

#include "TFPickupableActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"


ATFPickupableActor::ATFPickupableActor()
{
	PrimaryActorTick.bCanEverTick = true;
	InteractionText = FText::FromString("Pick Up");
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

void ATFPickupableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool ATFPickupableActor::Interact_Implementation(APawn* InstigatorPawn)
{
	bool bSuccess = OnPickup_Implementation(InstigatorPawn);

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

FInteractionData ATFPickupableActor::GetInteractionData_Implementation(APawn* InstigatorPawn) const
{
	FInteractionData Data = Super::GetInteractionData_Implementation(InstigatorPawn);

	if (!ItemData.ItemName.IsEmpty())
	{
		Data.InteractionText = FText::Format(
			FText::FromString("Pick Up {0}"),
			ItemData.ItemName
		);
	}

	if (ItemData.bIsStackable && ItemData.Quantity > 1)
	{
		Data.SecondaryText = FText::Format(
			FText::FromString("x{0}"),
			FText::AsNumber(ItemData.Quantity)
		);
	}

	return Data;
}

bool ATFPickupableActor::OnPickup_Implementation(APawn* Picker)
{
	if (!CanPickup_Implementation(Picker))
	{
		OnPickupFailed_Implementation(Picker, FText::FromString("Cannot pickup item"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Picked up item: %s (x%d)"), *ItemData.ItemName.ToString(), ItemData.Quantity);

	return true;
}

FItemData ATFPickupableActor::GetItemData_Implementation() const
{
	return ItemData;
}

bool ATFPickupableActor::CanPickup_Implementation(APawn* Picker) const
{
	if (!Picker)
	{
		return false;
	}

	return true;
}

void ATFPickupableActor::OnPickupFailed_Implementation(APawn* Picker, const FText& Reason)
{
	UE_LOG(LogTemp, Warning, TEXT("Pickup failed: %s"), *Reason.ToString());

	OnItemPickupFailed(Picker, Reason);
}

bool ATFPickupableActor::ShouldDestroyOnPickup_Implementation() const
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
