// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TFBackpackIndicatorWidget.generated.h"

class UTFInventoryComponent;
class UTextBlock;

UCLASS()
class WIDGETS_API UTFBackpackIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

#pragma region Widget Bindings

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* StatusText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* DropHintText;

#pragma endregion Widget Bindings

#pragma region Settings

	UPROPERTY(EditAnywhere, Category = "Backpack")
	FText InventoryOpenText = FText::FromString("premi I per chiudere lo zaino");

	UPROPERTY(EditAnywhere, Category = "Backpack")
	FText InventoryClosedText = FText::FromString("premi I per aprire lo zaino");

	UPROPERTY(EditAnywhere, Category = "Backpack")
	FText DropBackpackText = FText::FromString("premi Z per lasciare lo zaino");

#pragma endregion Settings

private:

	UPROPERTY()
	UTFInventoryComponent* CachedInventoryComponent;

protected:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void InitializeInventoryComponent();
	void UpdateVisibility();
	void OnBackpackActivated();
	void OnBackpackDeactivated();

	UFUNCTION()
	void OnInventoryToggled(bool bIsOpen);

public:

	void SetInventoryComponent(UTFInventoryComponent* NewInventoryComponent);
};
