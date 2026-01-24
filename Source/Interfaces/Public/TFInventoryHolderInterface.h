// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TFPickupableInterface.h"
#include "TFInventoryHolderInterface.generated.h"

UINTERFACE(MinimalAPI)
class UTFInventoryHolderInterface : public UInterface
{
	GENERATED_BODY()
};

class INTERFACES_API ITFInventoryHolderInterface
{
	GENERATED_BODY()

public:

	virtual bool HasBackpack() const { return false; }
	virtual bool ActivateBackpack(int32 Slots, float WeightLimit) { return false; }
	virtual void SetPendingBackpackActor(AActor* Actor) {}
	virtual bool AddItem(const FItemData& Item) { return false; }
	virtual bool RemoveItem(FName ItemID) { return false; }
	virtual bool HasItem(FName ItemID) const { return false; }
	virtual bool HasSpaceForItem(const FItemData& Item) const { return false; }
	virtual bool CanCarryWeight(float AdditionalWeight) const { return false; }
	virtual int32 GetFreeSlots() const { return 0; }
	virtual float GetRemainingCapacity() const { return 0.0f; }
};
