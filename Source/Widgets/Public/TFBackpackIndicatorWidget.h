// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TFBackpackIndicatorWidget.generated.h"

class UTFInventoryComponent;
class UImage;
class UTextBlock;

UCLASS()
class WIDGETS_API UTFBackpackIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

#pragma region Widget Bindings

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* BackpackIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* StatusText;

#pragma endregion Widget Bindings

#pragma region Settings

	UPROPERTY(EditAnywhere, Category = "Backpack")
	FText EquippedText = FText::FromString("Backpack Equipped");

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

public:

	void SetInventoryComponent(UTFInventoryComponent* NewInventoryComponent);
};
