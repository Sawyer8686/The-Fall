// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TFPickupableInterface.generated.h"

/**
 * Item data for pickupable objects
 * This will be expanded when inventory system is created
 */
USTRUCT(BlueprintType)
struct FItemData
{
	GENERATED_BODY()

	/** Unique identifier for this item type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ItemID = NAME_None;

	/** Display name of the item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText ItemName = FText::FromString("Item");

	/** Item description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText ItemDescription = FText::GetEmpty();

	/** Icon to display in inventory */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UTexture2D* ItemIcon = nullptr;

	/** Item mesh for preview */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UStaticMesh* ItemMesh = nullptr;

	/** Quantity of this item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 Quantity = 1;

	/** Can this item stack in inventory */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	bool bIsStackable = false;

	/** Maximum stack size (if stackable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 MaxStackSize = 99;

	/** Item weight (for inventory management) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float Weight = 1.0f;

	/** Item value (for trading) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
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

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UTFPickupableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for objects that can be picked up and added to inventory
 * Inherits from Interactable - pickup is a specific type of interaction
 */
class INTERFACES_API ITFPickupableInterface
{
	GENERATED_BODY()

public:

	/**
	 * Called when this item is picked up by player
	 * @param Picker - The pawn picking up this item
	 * @return True if pickup was successful
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pickup")
	bool OnPickup(APawn* Picker);
	virtual bool OnPickup_Implementation(APawn* Picker) { return false; }

	/**
	 * Get item data for this pickupable object
	 * @return Item data to be added to inventory
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pickup")
	FItemData GetItemData() const;
	virtual FItemData GetItemData_Implementation() const { return FItemData(); }

	/**
	 * Check if this item can currently be picked up
	 * @param Picker - The pawn attempting to pick up
	 * @return True if pickup is possible (inventory space, etc.)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pickup")
	bool CanPickup(APawn* Picker) const;
	virtual bool CanPickup_Implementation(APawn* Picker) const { return true; }

	/**
	 * Called when pickup fails (inventory full, etc.)
	 * @param Picker - The pawn who failed to pick up
	 * @param Reason - Reason for failure
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pickup")
	void OnPickupFailed(APawn* Picker, const FText& Reason);
	virtual void OnPickupFailed_Implementation(APawn* Picker, const FText& Reason) {}

	/**
	 * Should this item be destroyed after successful pickup
	 * @return True if actor should be destroyed (default for most items)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pickup")
	bool ShouldDestroyOnPickup() const;
	virtual bool ShouldDestroyOnPickup_Implementation() const { return true; }
};
