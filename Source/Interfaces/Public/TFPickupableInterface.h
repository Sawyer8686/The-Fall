// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Sound/SoundBase.h"
#include "TFPickupableInterface.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Generic     UMETA(DisplayName = "Generic Item"),
	Key         UMETA(DisplayName = "Key"),
	Consumable  UMETA(DisplayName = "Consumable"),
	Food        UMETA(DisplayName = "Food"),
	Beverage    UMETA(DisplayName = "Beverage"),
	Weapon      UMETA(DisplayName = "Weapon"),
	Ammo        UMETA(DisplayName = "Ammunition"),
	Document    UMETA(DisplayName = "Document"),
	Quest       UMETA(DisplayName = "Quest Item"),
	Backpack    UMETA(DisplayName = "Backpack")
};

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
	float Weight = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Item")
	int32 Value = 10;

	UPROPERTY(EditAnywhere, Category = "Item|Key")
	FName KeyID = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Item|Consumable")
	float HungerRestore = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Item|Consumable")
	float ThirstRestore = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Item|Backpack")
	int32 BackpackSlots = 5;

	UPROPERTY(EditAnywhere, Category = "Item|Backpack")
	float BackpackWeightLimit = 25.0f;

	FItemData()
		: ItemID(NAME_None)
		, ItemType(EItemType::Generic)
		, ItemName(FText::FromString("Item"))
		, ItemDescription(FText::GetEmpty())
		, ItemIcon(nullptr)
		, ItemMesh(nullptr)
		, PickupSound(nullptr)
		, Weight(1.0f)
		, Value(10)
		, KeyID(NAME_None)
		, HungerRestore(0.0f)
		, ThirstRestore(0.0f)
		, BackpackSlots(5)
		, BackpackWeightLimit(25.0f)
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
