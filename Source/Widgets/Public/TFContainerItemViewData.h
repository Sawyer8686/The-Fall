// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TFPickupableInterface.h"
#include "TFContainerItemViewData.generated.h"

class UTFContainerWidget;

UENUM()
enum class EContainerItemSource : uint8
{
	Container,
	Inventory
};

UCLASS()
class WIDGETS_API UTFContainerItemViewData : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY()
	FItemData ItemData;

	UPROPERTY()
	EContainerItemSource Source;

	UPROPERTY()
	TWeakObjectPtr<UTFContainerWidget> OwnerWidget;
};
