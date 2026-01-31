// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Engine/StaticMesh.h"
#include "TFPickupableInterface.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
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
	EItemType ItemType = EItemType::Food;

	UPROPERTY(EditAnywhere, Category = "Item")
	FText ItemName = FText::FromString("Item");

	UPROPERTY(EditAnywhere, Category = "Item")
	FText ItemDescription = FText::GetEmpty();

	UPROPERTY(EditAnywhere, Category = "Item")
	float Weight = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Item|Consumable")
	float HungerRestore = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Item|Consumable")
	float ThirstRestore = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Item|Backpack")
	int32 BackpackSlots = 5;

	UPROPERTY(EditAnywhere, Category = "Item|Backpack")
	float BackpackWeightLimit = 25.0f;

	UPROPERTY()
	UStaticMesh* ItemMesh = nullptr;

	UPROPERTY()
	FVector ItemMeshScale = FVector::OneVector;

	UPROPERTY()
	float MaxInteractionDistance = 500.0f;

	FItemData()
		: ItemID(NAME_None)
		, ItemType(EItemType::Food)
		, ItemName(FText::FromString("Item"))
		, ItemDescription(FText::GetEmpty())
		, Weight(1.0f)
		, HungerRestore(0.0f)
		, ThirstRestore(0.0f)
		, BackpackSlots(5)
		, BackpackWeightLimit(25.0f)
		, ItemMesh(nullptr)
		, ItemMeshScale(FVector::OneVector)
		, MaxInteractionDistance(500.0f)
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
