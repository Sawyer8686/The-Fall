// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TFPickupableInterface.h"
#include "TFInventoryComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnBackpackActivated);
DECLARE_MULTICAST_DELEGATE(FOnBackpackDeactivated);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemAdded, const FItemData&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemRemoved, FName);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryChanged, float, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventoryFull, const FText&);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INVENTORY_API UTFInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

private:

#pragma region Backpack State

	UPROPERTY(VisibleAnywhere, Category = "Inventory|Backpack")
	bool bHasBackpack = false;

	UPROPERTY(VisibleAnywhere, Category = "Inventory|Backpack")
	int32 BackpackSlots = 0;

	UPROPERTY(VisibleAnywhere, Category = "Inventory|Backpack")
	float BackpackWeightLimit = 0.0f;

#pragma endregion Backpack State

#pragma region Inventory State

	UPROPERTY(VisibleAnywhere, Category = "Inventory|Items")
	TArray<FItemData> Items;

	UPROPERTY(VisibleAnywhere, Category = "Inventory|Items")
	float CurrentWeight = 0.0f;

#pragma endregion Inventory State

protected:

	virtual void BeginPlay() override;

public:

	UTFInventoryComponent();

#pragma region Delegates

	FOnBackpackActivated OnBackpackActivated;
	FOnBackpackDeactivated OnBackpackDeactivated;
	FOnItemAdded OnItemAdded;
	FOnItemRemoved OnItemRemoved;
	FOnInventoryChanged OnInventoryChanged;
	FOnInventoryFull OnInventoryFull;

#pragma endregion Delegates

#pragma region Backpack Management

	bool ActivateBackpack(int32 Slots, float WeightLimit);
	TArray<FItemData> DeactivateBackpack();
	void RestoreItems(const TArray<FItemData>& ItemsToRestore);
	bool HasBackpack() const { return bHasBackpack; }
	int32 GetBackpackSlots() const { return BackpackSlots; }
	float GetBackpackWeightLimit() const { return BackpackWeightLimit; }

#pragma endregion Backpack Management

#pragma region Item Management

	bool AddItem(const FItemData& Item);
	bool RemoveItem(FName ItemID);
	bool HasItem(FName ItemID) const;
	const FItemData* GetItem(FName ItemID) const;
	const TArray<FItemData>& GetItems() const { return Items; }

#pragma endregion Item Management

#pragma region Capacity Queries

	bool HasSpaceForItem(const FItemData& Item) const;
	bool CanCarryWeight(float AdditionalWeight) const;
	int32 GetFreeSlots() const;
	int32 GetUsedSlots() const { return Items.Num(); }
	float GetRemainingCapacity() const;
	float GetCurrentWeight() const { return CurrentWeight; }
	float GetWeightPercent() const;

#pragma endregion Capacity Queries
};
