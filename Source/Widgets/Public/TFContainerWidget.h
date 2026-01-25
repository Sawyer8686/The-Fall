// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TFPickupableInterface.h"
#include "TFContainerInterface.h"
#include "TFContainerWidget.generated.h"

class UTFInventoryComponent;
class UTFContainerItemViewData;
class UListView;
class UTextBlock;
class UButton;

UCLASS()
class WIDGETS_API UTFContainerWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

#pragma region Widget Bindings

	UPROPERTY(meta = (BindWidget))
	UListView* ContainerListView;

	UPROPERTY(meta = (BindWidget))
	UListView* InventoryListView;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ContainerNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ContainerSlotsText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* InventorySlotsText;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* CloseButton;

#pragma endregion Widget Bindings

private:

	ITFContainerInterface* CachedContainer = nullptr;

	UPROPERTY()
	UTFInventoryComponent* CachedInventoryComponent;

	UPROPERTY()
	TArray<UTFContainerItemViewData*> ContainerListItems;

	UPROPERTY()
	TArray<UTFContainerItemViewData*> InventoryListItems;

protected:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void InitializeInventoryComponent();
	void PopulateContainerList();
	void PopulateInventoryList();
	void UpdateContainerSlotsDisplay();
	void UpdateInventorySlotsDisplay();

	void OnContainerChanged();
	void OnInventoryItemAdded(const FItemData& Item);
	void OnInventoryItemRemoved(FName ItemID);

	UFUNCTION()
	void OnCloseClicked();

public:

	void SetContainerSource(ITFContainerInterface* Container);
	void RefreshDisplay();
	void TakeItem(FName ItemID);
	void DepositItem(FName ItemID);
};
