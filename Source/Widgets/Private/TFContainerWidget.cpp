// Copyright TF Project. All Rights Reserved.

#include "TFContainerWidget.h"
#include "TFContainerItemViewData.h"
#include "TFInventoryComponent.h"
#include "TFPlayerCharacter.h"

#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UTFContainerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UTFContainerWidget::OnCloseClicked);
	}

	InitializeInventoryComponent();

	// Initialize container from static context (set by the actor before widget creation)
	if (FTFContainerContext::ActiveContainer)
	{
		SetContainerSource(FTFContainerContext::ActiveContainer);
	}
}

void UTFContainerWidget::NativeDestruct()
{
	if (CachedContainer)
	{
		CachedContainer->GetOnContainerChanged().RemoveAll(this);
	}

	if (CachedInventoryComponent)
	{
		CachedInventoryComponent->OnItemAdded.RemoveAll(this);
		CachedInventoryComponent->OnItemRemoved.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UTFContainerWidget::InitializeInventoryComponent()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		return;
	}

	ATFPlayerCharacter* Character = Cast<ATFPlayerCharacter>(PlayerPawn);
	if (!Character)
	{
		return;
	}

	CachedInventoryComponent = Character->GetInventoryComponent();
	if (CachedInventoryComponent)
	{
		CachedInventoryComponent->OnItemAdded.AddUObject(this, &UTFContainerWidget::OnInventoryItemAdded);
		CachedInventoryComponent->OnItemRemoved.AddUObject(this, &UTFContainerWidget::OnInventoryItemRemoved);
	}
}

void UTFContainerWidget::SetContainerSource(ITFContainerInterface* Container)
{
	if (CachedContainer)
	{
		CachedContainer->GetOnContainerChanged().RemoveAll(this);
	}

	CachedContainer = Container;

	if (CachedContainer)
	{
		CachedContainer->GetOnContainerChanged().AddUObject(this, &UTFContainerWidget::OnContainerChanged);

		if (ContainerNameText)
		{
			ContainerNameText->SetText(CachedContainer->GetContainerName());
		}
	}

	RefreshDisplay();
}

void UTFContainerWidget::PopulateContainerList()
{
	if (!ContainerListView)
	{
		return;
	}

	ContainerListView->ClearListItems();
	ContainerListItems.Empty();

	if (!CachedContainer)
	{
		return;
	}

	const TArray<FItemData>& Items = CachedContainer->GetContainerItems();

	for (const FItemData& Item : Items)
	{
		UTFContainerItemViewData* ViewData = NewObject<UTFContainerItemViewData>(this);
		ViewData->ItemData = Item;
		ViewData->Source = EContainerItemSource::Container;
		ViewData->OwnerWidget = this;

		ContainerListItems.Add(ViewData);
		ContainerListView->AddItem(ViewData);
	}
}

void UTFContainerWidget::PopulateInventoryList()
{
	if (!InventoryListView)
	{
		return;
	}

	InventoryListView->ClearListItems();
	InventoryListItems.Empty();

	if (!CachedInventoryComponent)
	{
		return;
	}

	const TArray<FItemData>& Items = CachedInventoryComponent->GetItems();

	for (const FItemData& Item : Items)
	{
		UTFContainerItemViewData* ViewData = NewObject<UTFContainerItemViewData>(this);
		ViewData->ItemData = Item;
		ViewData->Source = EContainerItemSource::Inventory;
		ViewData->OwnerWidget = this;

		InventoryListItems.Add(ViewData);
		InventoryListView->AddItem(ViewData);
	}
}

void UTFContainerWidget::UpdateContainerSlotsDisplay()
{
	if (!ContainerSlotsText || !CachedContainer)
	{
		return;
	}

	ContainerSlotsText->SetText(
		FText::FromString(FString::Printf(TEXT("%d / %d"),
			CachedContainer->GetContainerUsedSlots(), CachedContainer->GetMaxCapacity()))
	);
}

void UTFContainerWidget::UpdateInventorySlotsDisplay()
{
	if (!InventorySlotsText || !CachedInventoryComponent)
	{
		return;
	}

	const int32 UsedSlots = CachedInventoryComponent->GetUsedSlots();
	const int32 TotalSlots = CachedInventoryComponent->GetBackpackSlots();

	InventorySlotsText->SetText(
		FText::FromString(FString::Printf(TEXT("%d / %d"), UsedSlots, TotalSlots))
	);
}

void UTFContainerWidget::OnContainerChanged()
{
	PopulateContainerList();
	UpdateContainerSlotsDisplay();
}

void UTFContainerWidget::OnInventoryItemAdded(const FItemData& Item)
{
	PopulateInventoryList();
	UpdateInventorySlotsDisplay();
}

void UTFContainerWidget::OnInventoryItemRemoved(FName ItemID)
{
	PopulateInventoryList();
	UpdateInventorySlotsDisplay();
}

void UTFContainerWidget::OnCloseClicked()
{
	if (CachedContainer)
	{
		CachedContainer->CloseContainer();
	}
}

void UTFContainerWidget::RefreshDisplay()
{
	PopulateContainerList();
	PopulateInventoryList();
	UpdateContainerSlotsDisplay();
	UpdateInventorySlotsDisplay();
}

void UTFContainerWidget::TakeItem(FName ItemID)
{
	if (!CachedContainer || !CachedInventoryComponent)
	{
		return;
	}

	const FItemData* Item = CachedContainer->GetContainerItem(ItemID);
	if (!Item)
	{
		return;
	}

	if (!CachedInventoryComponent->HasSpaceForItem(*Item))
	{
		return;
	}

	FItemData ItemCopy = *Item;

	if (!CachedContainer->RemoveItemFromContainer(ItemID))
	{
		return;
	}

	CachedInventoryComponent->AddItem(ItemCopy);
}

void UTFContainerWidget::DepositItem(FName ItemID)
{
	if (!CachedContainer || !CachedInventoryComponent)
	{
		return;
	}

	if (!CachedContainer->ContainerHasSpace())
	{
		return;
	}

	const FItemData* Item = CachedInventoryComponent->GetItem(ItemID);
	if (!Item)
	{
		return;
	}

	FItemData ItemCopy = *Item;

	if (!CachedInventoryComponent->RemoveItem(ItemID))
	{
		return;
	}

	CachedContainer->AddItemToContainer(ItemCopy);
}
