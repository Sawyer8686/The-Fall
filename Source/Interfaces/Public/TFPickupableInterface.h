// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Sound/SoundBase.h"
#include "TFPickupableInterface.generated.h"

/**
 * Enum representing different types of pickupable items
 */
UENUM(BlueprintType)
enum class EItemType : uint8
{
	Generic     UMETA(DisplayName = "Generic Item"),
	Key         UMETA(DisplayName = "Key"),
	Consumable  UMETA(DisplayName = "Consumable"),
	Weapon      UMETA(DisplayName = "Weapon"),
	Ammo        UMETA(DisplayName = "Ammunition"),
	Document    UMETA(DisplayName = "Document"),
	Quest       UMETA(DisplayName = "Quest Item")
};

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
	EItemType ItemType = EItemType::Generic;

	UPROPERTY(EditAnywhere, Category = "Item")
	FText ItemName = FText::FromString("Item");

	UPROPERTY(EditAnywhere, Category = "Item")
	FText ItemDescription = FText::GetEmpty();

	UPROPERTY(EditAnywhere, Category = "Item")
	UTexture2D* ItemIcon = nullptr;

	UPROPERTY(EditAnywhere, Category = "Item")
	UStaticMesh* ItemMesh = nullptr;

	UPROPERTY(EditAnywhere, Category = "Item|Audio")
	USoundBase* PickupSound = nullptr;

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

	/** Key ID used only when ItemType == Key. Used to match with door RequiredKeyID */
	UPROPERTY(EditAnywhere, Category = "Item|Key")
	FName KeyID = NAME_None;

	FItemData()
		: ItemID(NAME_None)
		, ItemType(EItemType::Generic)
		, ItemName(FText::FromString("Item"))
		, ItemDescription(FText::GetEmpty())
		, ItemIcon(nullptr)
		, ItemMesh(nullptr)
		, PickupSound(nullptr)
		, Quantity(1)
		, bIsStackable(false)
		, MaxStackSize(99)
		, Weight(1.0f)
		, Value(10)
		, KeyID(NAME_None)
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
