// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TFPickupableInterface.h"
#include "TFInventoryItemViewData.generated.h"

class UTFInventoryWidget;

/**
 * Data object for a single inventory ListView row.
 * Each instance wraps one FItemData and a back-pointer to the owning widget.
 */
UCLASS()
class WIDGETS_API UTFInventoryItemViewData : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY()
	FItemData ItemData;

	UPROPERTY()
	TWeakObjectPtr<UTFInventoryWidget> OwnerWidget;
};
