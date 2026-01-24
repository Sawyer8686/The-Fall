// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "TFContainerItemEntryWidget.generated.h"

class UTextBlock;
class UButton;
class UTFContainerItemViewData;

UCLASS()
class WIDGETS_API UTFContainerItemEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemNameText;

	UPROPERTY(meta = (BindWidget))
	UButton* ActionButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ActionButtonText;

	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

private:

	UPROPERTY()
	UTFContainerItemViewData* CachedViewData;

	UFUNCTION()
	void OnActionClicked();
};
