// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TFItemActionHandler.generated.h"

class UTFInventoryWidget;

UCLASS()
class UTFItemActionHandler : public UObject
{
	GENERATED_BODY()

public:

	FName ItemID;

	UPROPERTY()
	TWeakObjectPtr<UTFInventoryWidget> OwnerWidget;

	UFUNCTION()
	void OnExamineClicked();

	UFUNCTION()
	void OnDiscardClicked();
};
