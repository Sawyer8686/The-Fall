// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "TFInventoryItemEntryWidget.generated.h"

class UTextBlock;
class UButton;
class UTFInventoryItemViewData;

/**
 * Entry widget for the inventory ListView.
 * Layout: item name on top, Esamina/Scarta buttons below.
 * Designed for use with UListView and UTFInventoryItemViewData.
 */
UCLASS()
class WIDGETS_API UTFInventoryItemEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemNameText;

	UPROPERTY(meta = (BindWidget))
	UButton* ExamineButton;

	UPROPERTY(meta = (BindWidget))
	UButton* DiscardButton;

	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

private:

	UPROPERTY()
	UTFInventoryItemViewData* CachedViewData;

	UFUNCTION()
	void OnExamineClicked();

	UFUNCTION()
	void OnDiscardClicked();
};
