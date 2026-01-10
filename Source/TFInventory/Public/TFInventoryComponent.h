#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FInventorySlot.h"
#include "TFInventoryComponent.generated.h"

struct FTFItemsData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TFINVENTORY_API UTFInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float MaxCarryWeight = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float CurrentCarryWeight = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 TotalNumberOfSlots = 10;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FInventorySlot> InventoryContents;

	void DropItemToWorldAtIndex(int32 Index);

	// Helpers
	int32 FindFirstEmptySlot() const;
	int32 AddToExistingStacks(const FTFItemsData& ItemData, int32 Quantity, float Durability);
	int32 AddToEmptySlots(const FTFItemsData& ItemData, int32 Quantity, float Durability);
	int32 ClampByWeight(const FTFItemsData& ItemData, int32 RequestedQuantity) const;

	void BroadcastInventoryChanged();

protected:

	virtual void BeginPlay() override;
	virtual void UpdateWeight();
	virtual void ResizeInventory();

public:

	UTFInventoryComponent();

	UPROPERTY(BlueprintAssignable)
	FOnInventoryChanged OnInventoryChanged;

	UFUNCTION(BlueprintCallable)
	int32 AddItemToInventory(const FTFItemsData& ItemData, int32 Quantity, float Durability = -1.0f);

	UFUNCTION(BlueprintCallable)
	bool RemoveItemFromInventory(FName ItemID, int32 Quantity);

	UFUNCTION(BlueprintCallable)
	bool UpdateInventorySlotCount(int32 NewSlots);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetCarryWeightPercentile() const { return (MaxCarryWeight <= 0.0f) ? 0.0f : (CurrentCarryWeight / MaxCarryWeight); }

	UFUNCTION(BlueprintCallable)
	int32 AddItemToSlot(const FTFItemsData& ItemData, int32 Quantity, float Durability = -1.0f, int32 Index = -1);

	// (Opzionale ma utile per UI)
	UFUNCTION(BlueprintCallable, BlueprintPure)
	const TArray<FInventorySlot>& GetInventoryContents() const { return InventoryContents; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetTotalSlots() const { return TotalNumberOfSlots; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetCurrentCarryWeight() const { return CurrentCarryWeight; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetMaxCarryWeight() const { return MaxCarryWeight; }
};
