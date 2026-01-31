// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TFBackpackConfirmWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class WIDGETS_API UTFBackpackConfirmWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* QuestionText;

	UPROPERTY(meta = (BindWidget))
	UButton* YesButton;

	UPROPERTY(meta = (BindWidget))
	UButton* NoButton;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:

	UFUNCTION()
	void OnYesClicked();

	UFUNCTION()
	void OnNoClicked();

public:

	/** Set the backpack info to display */
	void SetBackpackInfo(int32 Slots, float WeightLimit);
};
