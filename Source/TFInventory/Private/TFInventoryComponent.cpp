#include "TFInventoryComponent.h"
#include "IntVectorTypes.h"
#include "TFItemsData.h"


UTFInventoryComponent::UTFInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;	
}

void UTFInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	ResizeInventory();
	UpdateWeight();
}

void UTFInventoryComponent::DropItemToWorldAtIndex(int32 Index)
{
	if (!GetWorld())
	{
		return;
	}

	if (!InventoryContents.IsValidIndex(Index))
	{
		return;
	}

	const FInventorySlot& Slot = InventoryContents[Index];
	if (!Slot.IsValid())
	{
		return;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("DropItemToWorldAtIndex: TODO spawn pickup for ItemID=%s Qty=%d Dur=%.2f Weight=%.2f"),
		*Slot.ItemID.ToString(),
		Slot.Quantity,
		Slot.CurrentDurability,
		Slot.ItemWeight
	);

	// TODO: qui va lo spawn del pickup.
	// In single player la policy consigliata è:
	// - spawn actor pickup davanti al player / owner
	// - impostare i dati (ItemID, Quantity, Durability)
}

int32 UTFInventoryComponent::FindFirstEmptySlot() const
{
	for (int32 i = 0; i < InventoryContents.Num(); ++i)
	{
		if (!InventoryContents[i].IsValid())
		{
			return i;
		}
	}
	return INDEX_NONE;
}

int32 UTFInventoryComponent::ClampByWeight(const FTFItemsData& ItemData, int32 RequestedQuantity) const
{
	if (RequestedQuantity <= 0)
	{
		return 0;
	}

	// Peso disabilitato o item a peso zero: nessun clamp
	if (MaxCarryWeight <= 0.0f || ItemData.SingleItemWeight <= 0.0f)
	{
		return RequestedQuantity;
	}

	const float RemainingWeight = MaxCarryWeight - CurrentCarryWeight;
	if (RemainingWeight <= 0.0f)
	{
		return 0;
	}

	const int32 MaxByWeight = FMath::FloorToInt(RemainingWeight / ItemData.SingleItemWeight);
	return FMath::Clamp(RequestedQuantity, 0, MaxByWeight);
}

int32 UTFInventoryComponent::AddToExistingStacks(const FTFItemsData& ItemData, int32 Quantity, float Durability)
{
	int32 Remaining = Quantity;

	for (FInventorySlot& Slot : InventoryContents)
	{
		if (Remaining <= 0)
		{
			break;
		}

		if (!Slot.IsValid())
		{
			continue;
		}

		if (Slot.ItemID != ItemData.ItemID)
		{
			continue;
		}

		if (Slot.Quantity >= ItemData.ItemMaxStackSize)
		{
			continue;
		}

		const int32 Space = ItemData.ItemMaxStackSize - Slot.Quantity;
		const int32 ToAdd = FMath::Min(Space, Remaining);

		Slot.Quantity += ToAdd;
		Remaining -= ToAdd;
	}

	return Remaining;
}

int32 UTFInventoryComponent::AddToEmptySlots(const FTFItemsData& ItemData, int32 Quantity, float Durability)
{
	int32 Remaining = Quantity;

	while (Remaining > 0)
	{
		const int32 EmptyIndex = FindFirstEmptySlot();
		if (EmptyIndex == INDEX_NONE)
		{
			break;
		}

		const int32 ToAdd = FMath::Min(ItemData.ItemMaxStackSize, Remaining);
		InventoryContents[EmptyIndex].SetItemData(ItemData.ItemID, ToAdd, Durability, ItemData.SingleItemWeight);
		Remaining -= ToAdd;
	}

	return Remaining;
}

void UTFInventoryComponent::BroadcastInventoryChanged()
{
	OnInventoryChanged.Broadcast();
}

int32 UTFInventoryComponent::AddItemToInventory(const FTFItemsData& ItemData, int32 Quantity, float Durability)
{
	if (Quantity <= 0)
	{
		return Quantity; // nulla da aggiungere
	}

	if (!ItemData.IsValid())
	{
		return Quantity; // item non valido => nulla aggiunto
	}

	// clamp per peso (single player)
	const int32 AllowedByWeight = ClampByWeight(ItemData, Quantity);
	if (AllowedByWeight <= 0)
	{
		return Quantity; // tutto rimane non aggiunto
	}

	int32 Remaining = AllowedByWeight;

	// 1) riempi gli stack esistenti
	Remaining = AddToExistingStacks(ItemData, Remaining, Durability);

	// 2) crea nuovi stack in slot vuoti
	Remaining = AddToEmptySlots(ItemData, Remaining, Durability);

	UpdateWeight();
	BroadcastInventoryChanged();

	// ritorna quantità non aggiunta rispetto alla richiesta originale:
	// - se weight clamp ha tagliato, la parte tagliata resta "non aggiunta"
	const int32 NotAddedByWeight = Quantity - AllowedByWeight;
	return Remaining + NotAddedByWeight;
}

bool UTFInventoryComponent::RemoveItemFromInventory(FName ItemID, int32 Quantity)
{
	if (ItemID.IsNone() || Quantity <= 0)
	{
		return false;
	}

	int32 RemainingToRemove = Quantity;

	// Strategia: rimuovo dai primi stack trovati; se preferisci LIFO o “damaged first” lo adattiamo.
	for (FInventorySlot& Slot : InventoryContents)
	{
		if (RemainingToRemove <= 0)
		{
			break;
		}

		if (!Slot.IsValid())
		{
			continue;
		}

		if (Slot.ItemID != ItemID)
		{
			continue;
		}

		const int32 RemoveFromThis = FMath::Min(Slot.Quantity, RemainingToRemove);
		Slot.Quantity -= RemoveFromThis;
		RemainingToRemove -= RemoveFromThis;

		if (Slot.Quantity <= 0)
		{
			Slot.ClearItemData();
		}
	}

	const bool bRemovedAll = (RemainingToRemove <= 0);

	// In single player aggiornare sempre peso e UI: anche una rimozione parziale è una mutazione.
	UpdateWeight();
	BroadcastInventoryChanged();

	return bRemovedAll;
}

bool UTFInventoryComponent::UpdateInventorySlotCount(int32 NewSlots)
{
	if (NewSlots <= 0)
	{
		return false;
	}

	TotalNumberOfSlots = NewSlots;
	ResizeInventory();
	UpdateWeight();
	BroadcastInventoryChanged();
	return true;
}

int32 UTFInventoryComponent::AddItemToSlot(const FTFItemsData& ItemData, int32 Quantity, float Durability, int32 Index)
{
	if (Index < 0 || Index >= TotalNumberOfSlots)
	{
		return AddItemToInventory(ItemData, Quantity, Durability);
	}

	if (Quantity <= 0 || !ItemData.IsValid())
	{
		return Quantity;
	}

	// clamp per peso (single player)
	const int32 AllowedByWeight = ClampByWeight(ItemData, Quantity);
	if (AllowedByWeight <= 0)
	{
		return Quantity;
	}

	int32 Remaining = AllowedByWeight;

	// Se lo slot è vuoto: inserisci lì
	if (!InventoryContents[Index].IsValid())
	{
		const int32 ToAdd = FMath::Min(ItemData.ItemMaxStackSize, Remaining);
		InventoryContents[Index].SetItemData(ItemData.ItemID, ToAdd, Durability, ItemData.SingleItemWeight);
		Remaining -= ToAdd;

		UpdateWeight();
		BroadcastInventoryChanged();

		const int32 NotAddedByWeight = Quantity - AllowedByWeight;
		return Remaining + NotAddedByWeight;
	}

	// Slot occupato: prova a spostare l’item esistente
	const int32 EmptyIndex = FindFirstEmptySlot();
	if (EmptyIndex != INDEX_NONE)
	{
		InventoryContents[EmptyIndex] = InventoryContents[Index];
		InventoryContents[Index].ClearItemData();
	}
	else
	{
		// Nessuno slot vuoto: drop del contenuto corrente
		DropItemToWorldAtIndex(Index);
		InventoryContents[Index].ClearItemData();
	}

	// Inserisci il nuovo item nello slot target
	{
		const int32 ToAdd = FMath::Min(ItemData.ItemMaxStackSize, Remaining);
		InventoryContents[Index].SetItemData(ItemData.ItemID, ToAdd, Durability, ItemData.SingleItemWeight);
		Remaining -= ToAdd;
	}

	// Se resta, fallback su inserimento generale
	if (Remaining > 0)
	{
		Remaining = AddItemToInventory(ItemData, Remaining, Durability);
	}

	UpdateWeight();
	BroadcastInventoryChanged();

	const int32 NotAddedByWeight = Quantity - AllowedByWeight;
	return Remaining + NotAddedByWeight;
}

void UTFInventoryComponent::ResizeInventory()
{
	// crescita
	if (InventoryContents.Num() < TotalNumberOfSlots)
	{
		InventoryContents.Reserve(TotalNumberOfSlots);
		while (InventoryContents.Num() < TotalNumberOfSlots)
		{
			InventoryContents.Add(FInventorySlot());
		}
		return;
	}

	// shrink: droppa gli item in eccesso in modo esplicito
	while (InventoryContents.Num() > TotalNumberOfSlots)
	{
		const int32 LastIndex = InventoryContents.Num() - 1;
		if (InventoryContents[LastIndex].IsValid())
		{
			DropItemToWorldAtIndex(LastIndex);
		}
		InventoryContents.RemoveAt(LastIndex);
	}
	
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


