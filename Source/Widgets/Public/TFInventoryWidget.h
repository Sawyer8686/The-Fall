// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TFPickupableInterface.h"
#include "TFInventoryWidget.generated.h"

class UTFInventoryComponent;
class UTFInventoryItemViewData;
class UListView;
class UTextBlock;
class UProgressBar;
class UVerticalBox;

UCLASS()
class WIDGETS_API UTFInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

#pragma region Widget Bindings

	UPROPERTY(meta = (BindWidget))
	UListView* ItemListView;

	UPROPERTY(meta = (BindWidgetOptional))
	UVerticalBox* KeychainContainer;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* WeightText;

	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* WeightBar;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* SlotsText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* DescriptionText;

#pragma endregion Widget Bindings

#pragma region Visual Settings

	UPROPERTY(EditAnywhere, Category = "Inventory|Colors")
	FLinearColor LowWeightColor = FLinearColor(0.2f, 0.8f, 0.2f);

	UPROPERTY(EditAnywhere, Category = "Inventory|Colors")
	FLinearColor MediumWeightColor = FLinearColor(0.8f, 0.6f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "Inventory|Colors")
	FLinearColor HighWeightColor = FLinearColor(0.8f, 0.2f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "Inventory|Colors", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MediumWeightThreshold = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Inventory|Colors", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HighWeightThreshold = 0.8f;

#pragma endregion Visual Settings

private:

	UPROPERTY()
	UTFInventoryComponent* CachedInventoryComponent;

	UPROPERTY()
	TArray<UTFInventoryItemViewData*> ListItems;

	FName CurrentExaminedItemID = NAME_None;

protected:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void InitializeInventoryComponent();
	void PopulateListView();
	void RebuildKeychainList();
	void UpdateWeightDisplay(float CurrentWeight, float MaxWeight);
	void UpdateSlotDisplay();
	void UpdateWeightColor(float WeightPercent);

	void OnItemAdded(const FItemData& Item);
	void OnItemRemoved(FName ItemID);
	void OnInventoryChanged(float CurrentWeight, float MaxWeight);

	UFUNCTION()
	void OnInventoryToggled(bool bIsOpen);

	void OnKeyCollectionChanged();

public:

	void SetInventoryComponent(UTFInventoryComponent* NewInventoryComponent);
	void RefreshDisplay();
	void ExamineItem(FName ItemID);
	void DiscardItem(FName ItemID);
	void ConsumeItem(FName ItemID);
};
