#pragma once

#include "CoreMinimal.h"
#include "FInventorySlot.generated.h"

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName ItemID = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int Quantity = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentDurability = -1.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ItemWeight = 0.0f;

	FInventorySlot() {}

	void SetItemData(const FName ItemID, const int Quantity, const float Durability, const float Weight)
	{
		this->ItemID = ItemID;
		this->Quantity = Quantity;
		this->CurrentDurability = Durability;
		this->ItemWeight = Weight;
	}
	
	FORCEINLINE void ClearItemData()
	{
		ItemID = NAME_None;
		Quantity = 0;
		CurrentDurability = -1.0;
		ItemWeight = 0.0f;
	}

	FORCEINLINE bool IsValid() const
	{
		return !ItemID.IsNone() && Quantity > 0;
	}


};