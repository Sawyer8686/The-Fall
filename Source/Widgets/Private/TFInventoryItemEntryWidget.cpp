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

	if (ConsumeButton)
	{
		ConsumeButton->OnClicked.AddDynamic(this, &UTFInventoryItemEntryWidget::OnConsumeClicked);
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

	UpdateConsumeButton();
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

void UTFInventoryItemEntryWidget::OnConsumeClicked()
{
	if (CachedViewData && CachedViewData->OwnerWidget.IsValid())
	{
		CachedViewData->OwnerWidget->ConsumeItem(CachedViewData->ItemData.ItemID);
	}
}

void UTFInventoryItemEntryWidget::UpdateConsumeButton()
{
	if (!ConsumeButton)
	{
		return;
	}

	if (!CachedViewData)
	{
		ConsumeButton->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const EItemType Type = CachedViewData->ItemData.ItemType;

	if (Type == EItemType::Food)
	{
		ConsumeButton->SetVisibility(ESlateVisibility::Visible);
		if (ConsumeButtonText)
		{
			ConsumeButtonText->SetText(FText::FromString(TEXT("Mangia")));
		}
	}
	else if (Type == EItemType::Beverage)
	{
		ConsumeButton->SetVisibility(ESlateVisibility::Visible);
		if (ConsumeButtonText)
		{
			ConsumeButtonText->SetText(FText::FromString(TEXT("Bevi")));
		}
	}
	else
	{
		ConsumeButton->SetVisibility(ESlateVisibility::Collapsed);
	}
}
