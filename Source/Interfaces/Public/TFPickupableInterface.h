// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TFPickupableInterface.generated.h"

/**
 * Item data for pickupable objects
 * This will be expanded when inventory system is created
 */
USTRUCT()
struct FItemData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Item")
	FName ItemID = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Item")
	FText ItemName = FText::FromString("Item");

	UPROPERTY(EditAnywhere, Category = "Item")
	FText ItemDescription = FText::GetEmpty();

	UPROPERTY(EditAnywhere, Category = "Item")
	UTexture2D* ItemIcon = nullptr;

	UPROPERTY(EditAnywhere, Category = "Item")
	UStaticMesh* ItemMesh = nullptr;

	UPROPERTY(EditAnywhere, Category = "Item")
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, Category = "Item")
	bool bIsStackable = false;

	UPROPERTY(EditAnywhere, Category = "Item")
	int32 MaxStackSize = 99;

	UPROPERTY(EditAnywhere, Category = "Item")
	float Weight = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Item")
	int32 Value = 10;

	FItemData()
		: ItemID(NAME_None)
		, ItemName(FText::FromString("Item"))
		, ItemDescription(FText::GetEmpty())
		, ItemIcon(nullptr)
		, ItemMesh(nullptr)
		, Quantity(1)
		, bIsStackable(false)
		, MaxStackSize(99)
		, Weight(1.0f)
		, Value(10)
	{
	}
};

UINTERFACE(MinimalAPI)
class UTFPickupableInterface : public UInterface
{
	GENERATED_BODY()
};

class INTERFACES_API ITFPickupableInterface
{
	GENERATED_BODY()

public:

	virtual bool OnPickup(APawn* Picker) { return false; }

	virtual FItemData GetItemData() const { return FItemData(); }

	virtual bool CanPickup(APawn* Picker) const { return true; }

	virtual void OnPickupFailed(APawn* Picker, const FText& Reason) {}

	virtual bool ShouldDestroyOnPickup() const { return true; }
};
