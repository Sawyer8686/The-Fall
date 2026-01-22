// Copyright TF Project. All Rights Reserved.

#include "TFInventoryComponent.h"
#include "TFTypes.h"

UTFInventoryComponent::UTFInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTFInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UTFInventoryComponent::ActivateBackpack(int32 Slots, float WeightLimit)
{
	if (bHasBackpack)
	{
		UE_LOG(LogTFItem, Warning, TEXT("UTFInventoryComponent: Backpack already active"));
		return false;
	}

	bHasBackpack = true;
	BackpackSlots = FMath::Max(1, Slots);
	BackpackWeightLimit = FMath::Max(1.0f, WeightLimit);

	UE_LOG(LogTFItem, Log, TEXT("UTFInventoryComponent: Backpack activated (Slots: %d, Weight Limit: %.1f)"),
		BackpackSlots, BackpackWeightLimit);

	OnBackpackActivated.Broadcast();
	OnInventoryChanged.Broadcast(CurrentWeight, BackpackWeightLimit);

	return true;
}

bool UTFInventoryComponent::AddItem(const FItemData& Item)
{
	if (!bHasBackpack)
	{
		UE_LOG(LogTFItem, Warning, TEXT("UTFInventoryComponent: Cannot add item - no backpack"));
		OnInventoryFull.Broadcast(FText::FromString("No backpack equipped"));
		return false;
	}

	if (!HasSpaceForItem(Item))
	{
		FText Reason;
		if (GetUsedSlots() >= BackpackSlots)
		{
			Reason = FText::FromString("Inventory full - no slots available");
		}
		else
		{
			Reason = FText::FromString("Item too heavy");
		}

		UE_LOG(LogTFItem, Warning, TEXT("UTFInventoryComponent: Cannot add item - %s"), *Reason.ToString());
		OnInventoryFull.Broadcast(Reason);
		return false;
	}

	Items.Add(Item);
	CurrentWeight += Item.Weight;

	UE_LOG(LogTFItem, Log, TEXT("UTFInventoryComponent: Added item '%s' (%.1f kg)"),
		*Item.ItemName.ToString(), Item.Weight);

	OnItemAdded.Broadcast(Item);
	OnInventoryChanged.Broadcast(CurrentWeight, BackpackWeightLimit);

	return true;
}

bool UTFInventoryComponent::RemoveItem(FName ItemID, int32 Quantity)
{
	if (ItemID.IsNone() || Quantity <= 0)
	{
		return false;
	}

	for (int32 i = Items.Num() - 1; i >= 0; --i)
	{
		if (Items[i].ItemID == ItemID)
		{
			float WeightRemoved = Items[i].Weight;
			Items.RemoveAt(i);
			CurrentWeight -= WeightRemoved;

			UE_LOG(LogTFItem, Log, TEXT("UTFInventoryComponent: Removed item '%s'"), *ItemID.ToString());

			OnItemRemoved.Broadcast(ItemID, 1);
			OnInventoryChanged.Broadcast(CurrentWeight, BackpackWeightLimit);
			return true;
		}
	}

	return false;
}

bool UTFInventoryComponent::HasItem(FName ItemID) const
{
	if (ItemID.IsNone())
	{
		return false;
	}

	for (const FItemData& Item : Items)
	{
		if (Item.ItemID == ItemID)
		{
			return true;
		}
	}
	return false;
}

const FItemData* UTFInventoryComponent::GetItem(FName ItemID) const
{
	for (const FItemData& Item : Items)
	{
		if (Item.ItemID == ItemID)
		{
			return &Item;
		}
	}
	return nullptr;
}

bool UTFInventoryComponent::HasSpaceForItem(const FItemData& Item) const
{
	if (!bHasBackpack)
	{
		return false;
	}

	if (GetUsedSlots() >= BackpackSlots)
	{
		return false;
	}

	if (!CanCarryWeight(Item.Weight))
	{
		return false;
	}

	return true;
}

bool UTFInventoryComponent::CanCarryWeight(float AdditionalWeight) const
{
	if (!bHasBackpack)
	{
		return false;
	}

	return (CurrentWeight + AdditionalWeight) <= BackpackWeightLimit;
}

int32 UTFInventoryComponent::GetFreeSlots() const
{
	if (!bHasBackpack)
	{
		return 0;
	}

	return FMath::Max(0, BackpackSlots - GetUsedSlots());
}

float UTFInventoryComponent::GetRemainingCapacity() const
{
	if (!bHasBackpack)
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, BackpackWeightLimit - CurrentWeight);
}

float UTFInventoryComponent::GetWeightPercent() const
{
	if (!bHasBackpack || BackpackWeightLimit <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(CurrentWeight / BackpackWeightLimit, 0.0f, 1.0f);
}
