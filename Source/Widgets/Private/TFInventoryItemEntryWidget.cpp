// Copyright TF Project. All Rights Reserved.

#include "TFInventoryItemEntryWidget.h"
#include "TFInventoryItemViewData.h"
#include "TFInventoryWidget.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"

void UTFInventoryItemEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ExamineButton)
	{
		ExamineButton->OnClicked.AddDynamic(this, &UTFInventoryItemEntryWidget::OnExamineClicked);
	}

	if (DiscardButton)
	{
		DiscardButton->OnClicked.AddDynamic(this, &UTFInventoryItemEntryWidget::OnDiscardClicked);
	}
}

void UTFInventoryItemEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	CachedViewData = Cast<UTFInventoryItemViewData>(ListItemObject);
	if (!CachedViewData)
	{
		return;
	}

	if (ItemNameText)
	{
		const FItemData& Data = CachedViewData->ItemData;
		const FString DisplayText = FString::Printf(TEXT("%s  (%.1f kg)"), *Data.ItemName.ToString(), Data.Weight);
		ItemNameText->SetText(FText::FromString(DisplayText));
	}
}

void UTFInventoryItemEntryWidget::OnExamineClicked()
{
	if (CachedViewData && CachedViewData->OwnerWidget.IsValid())
	{
		CachedViewData->OwnerWidget->ExamineItem(CachedViewData->ItemData.ItemID);
	}
}

void UTFInventoryItemEntryWidget::OnDiscardClicked()
{
	if (CachedViewData && CachedViewData->OwnerWidget.IsValid())
	{
		CachedViewData->OwnerWidget->DiscardItem(CachedViewData->ItemData.ItemID);
	}
}
