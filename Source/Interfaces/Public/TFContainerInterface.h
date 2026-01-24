// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TFPickupableInterface.h"
#include "TFContainerInterface.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnContainerContentChanged);

UINTERFACE(MinimalAPI)
class UTFContainerInterface : public UInterface
{
	GENERATED_BODY()
};

class INTERFACES_API ITFContainerInterface
{
	GENERATED_BODY()

public:

	virtual const TArray<FItemData>& GetContainerItems() const = 0;
	virtual int32 GetMaxCapacity() const = 0;
	virtual int32 GetContainerUsedSlots() const = 0;
	virtual int32 GetContainerFreeSlots() const = 0;
	virtual bool ContainerHasSpace() const = 0;
	virtual bool AddItemToContainer(const FItemData& Item) = 0;
	virtual bool RemoveItemFromContainer(FName ItemID) = 0;
	virtual const FItemData* GetContainerItem(FName ItemID) const = 0;
	virtual FText GetContainerName() const = 0;
	virtual void CloseContainer() = 0;
	virtual FOnContainerContentChanged& GetOnContainerChanged() = 0;
};

/**
 * Static context for passing the active container reference from the actor to the widget
 * without requiring a direct module dependency between TFWorldActors and Widgets.
 */
struct INTERFACES_API FTFContainerContext
{
	static ITFContainerInterface* ActiveContainer;
};
