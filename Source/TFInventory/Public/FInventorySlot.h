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

	FInventorySlot() {}

	FORCEINLINE void Clear()
	{
		ItemID = NAME_None;
		Quantity = 0;
		CurrentDurability = -1.0;
	}

	FORCEINLINE bool IsValid() const
	{
		return ItemID != NAME_None && Quantity > 0;
	}


};