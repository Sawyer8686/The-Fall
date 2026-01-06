#include "TFInventoryComponent.h"
#include "TFItemsData.h"


UTFInventoryComponent::UTFInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;	
}

int UTFInventoryComponent::AddItemToInventory(const FTFItemsData& ItemData, const int Quantity, const float Durability)
{
	int NumberToAdd = Quantity;
	for (auto Item : InventoryContents)
	{
		if (!Item.IsValid() || ItemData.ItemID != Item.ItemID || Item.Quantity >= ItemData.ItemMaxStackSize)
		{
			continue;
		}
		int StackSpace = ItemData.ItemMaxStackSize - Item.Quantity;

		int ToAdd = FMath::Min(StackSpace, NumberToAdd);
		Item.Quantity += ToAdd;
		NumberToAdd -= ToAdd;

		if (NumberToAdd <= 0)
		{
			return NumberToAdd; //return number not added
		}

		for (auto Item : InventoryContents)
		{
			if (Item.IsValid())
			{
				continue;
			}
		}

		int ToAdd = FMath::Min(ItemData.ItemMaxStackSize, NumberToAdd);
		Item.SetItemData(ItemData.ItemID, ToAdd, Durability, ItemData.SingleItemWeight);
		NumberToAdd -= ToAdd;
		if (NumberToAdd <= 0)
		{
			return NumberToAdd; //return number not added
		}

	}


	return NumberToAdd; //return number not added
}

bool UTFInventoryComponent::RemoveItemFromInventory(const FName ItemID, const int Quantity)
{
	int NumberToRemove = Quantity;
	TArray<int> ItemAtIndex;
	for (int i = InventoryContents.Num() -1; i >= 0; i--)
	{
		if (InventoryContents[i].ItemID == ItemID)
		{
			ItemAtIndex.Add(i);
			int RemovedFromStack = FMath::Min(InventoryContents[i].Quantity, NumberToRemove);
			NumberToRemove -= RemovedFromStack;
			if(NumberToRemove <= 0)
			{
				break;
			}
		}
	}
	if(NumberToRemove > 0)
	{
		return false;
	}
	
	NumberToRemove = Quantity;
	for (const auto i : ItemAtIndex)
	{
		int StackSize = InventoryContents[i].Quantity;
		int ToRemove = FMath::Min(StackSize, NumberToRemove);
		InventoryContents[i].Quantity -= ToRemove;
		NumberToRemove -= ToRemove;

		if(!InventoryContents[i].IsValid())
		{
			InventoryContents[i].ClearItemData();
		}
		if (NumberToRemove <= 0)
		{
			break;
		}	
	}
	return true;
}

bool UTFInventoryComponent::UpdateInventorySlotCount(const int NewSlots)
{
	TotalNumberOfSlots = NewSlots;
	ResizeInventory();
	UpdateWeight();
	return true;
}

void UTFInventoryComponent::ResizeInventory()
{
	if (InventoryContents.Num() == TotalNumberOfSlots)
	{
		return;
	}

	if (InventoryContents.Num() < TotalNumberOfSlots)
	{
		while(InventoryContents.Num() < TotalNumberOfSlots)
		{
			InventoryContents.Add(FInventorySlot());
		}
		return;
	}

	while (InventoryContents.Num() > TotalNumberOfSlots)
	{
		FInventorySlot DroppedItem = InventoryContents.Pop();
		if (DroppedItem.IsValid())
		{
			//TODO: Drop extra item in the world? -> call to SpawnManager?
		}
		
	}
	
}

void UTFInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	ResizeInventory();
	UpdateWeight();
}

void UTFInventoryComponent::UpdateWeight()
{
	float tempWeight = 0.0f;
	for (const auto Item : InventoryContents)
	{
		if (!Item.IsValid())
		{
			continue;
		}

		tempWeight += (Item.Quantity * Item.ItemWeight);
	}
	CurrentCarryWeight = tempWeight;
}


