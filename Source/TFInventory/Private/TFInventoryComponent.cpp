#include "TFInventoryComponent.h"


UTFInventoryComponent::UTFInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;	
}

bool UTFInventoryComponent::UpdateInventorySlots(const int NewSlots)
{
	TotalNumberOfSlots = NewSlots;
	ResizeInventory();
	//Recalculate slots/weight
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
	//Set Array Size
	ResizeInventory();
}


