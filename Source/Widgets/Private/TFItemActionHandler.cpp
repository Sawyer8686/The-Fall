// Copyright TF Project. All Rights Reserved.

#include "TFItemActionHandler.h"
#include "TFInventoryWidget.h"

void UTFItemActionHandler::OnExamineClicked()
{
	if (OwnerWidget.IsValid())
	{
		OwnerWidget->ExamineItem(ItemID);
	}
}

void UTFItemActionHandler::OnDiscardClicked()
{
	if (OwnerWidget.IsValid())
	{
		OwnerWidget->DiscardItem(ItemID);
	}
}
