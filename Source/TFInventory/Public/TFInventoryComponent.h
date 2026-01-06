#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FInventorySlot.h"
#include "TFInventoryComponent.generated.h"

struct FTFItemsData;


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TFINVENTORY_API UTFInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float MaxCarryWeight = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float CurrentCarryWeight = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int TotalNumberOfSlots = 10;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FInventorySlot> InventoryContents;

	void DropItemToWorldAtIndex(const int index);

protected:

	virtual void BeginPlay() override;
	virtual void UpdateWeight();
	virtual void ResizeInventory();

public:

	UTFInventoryComponent();

	UFUNCTION(BlueprintCallable)
	int AddItemToInventory(const FTFItemsData& ItemData, const int Quantity, const float Durability = -1.0);

	UFUNCTION(BlueprintCallable)
	bool RemoveItemFromInventory(const FName ItemID, const int Quantity);

	UFUNCTION(BlueprintCallable)
	void TransferItemFromInventory() {};

	UFUNCTION(BlueprintCallable)
	bool UpdateInventorySlotCount(const int NewSlots);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetCarryWeightPercentile() const { return CurrentCarryWeight / MaxCarryWeight; }

	UFUNCTION(BlueprintCallable)
	int AddItemToSlot(const FTFItemsData& ItemData, const int Quantity, const float Durability = -1.0, const int index = -1);
		
};
