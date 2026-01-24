// Copyright TF Project. All Rights Reserved.

#include "TFContainerItemEntryWidget.h"
#include "TFContainerItemViewData.h"
#include "TFContainerWidget.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"

void UTFContainerItemEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ActionButton)
	{
		ActionButton->OnClicked.AddDynamic(this, &UTFContainerItemEntryWidget::OnActionClicked);
	}
}

void UTFContainerItemEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	CachedViewData = Cast<UTFContainerItemViewData>(ListItemObject);
	if (!CachedViewData)
	{
		return;
	}

	if (ItemNameText)
	{
		const FItemData& Data = CachedViewData->ItemData;
		ItemNameText->SetText(Data.ItemName);
	}

	if (ActionButtonText)
	{
		if (CachedViewData->Source == EContainerItemSource::Container)
		{
			ActionButtonText->SetText(FText::FromString(TEXT("Prendi")));
		}
		else
		{
			ActionButtonText->SetText(FText::FromString(TEXT("Deposita")));
		}
	}
}

void UTFContainerItemEntryWidget::OnActionClicked()
{
	if (!CachedViewData || !CachedViewData->OwnerWidget.IsValid())
	{
		return;
	}

	if (CachedViewData->Source == EContainerItemSource::Container)
	{
		CachedViewData->OwnerWidget->TakeItem(CachedViewData->ItemData.ItemID);
	}
	else
	{
		CachedViewData->OwnerWidget->DepositItem(CachedViewData->ItemData.ItemID);
	}
}
