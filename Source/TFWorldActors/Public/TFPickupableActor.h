// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TFInteractableActor.h"
#include "TFPickupableInterface.h"
#include "TFPickupableActor.generated.h"

UCLASS()
class TFWORLDACTORS_API ATFPickupableActor : public ATFInteractableActor, public ITFPickupableInterface
{
	GENERATED_BODY()

protected:

#pragma region Item Data

	UPROPERTY(EditAnywhere, Category = "Item", meta = (EditCondition = "!bUseDataDrivenConfig", EditConditionHides))
	FItemData ItemData;

#pragma endregion Item Data

#pragma region Pickup Settings

	UPROPERTY(EditAnywhere, Category = "Pickup", meta = (EditCondition = "!bUseDataDrivenConfig", EditConditionHides))
	bool bDestroyOnPickup = true;

	UPROPERTY(EditAnywhere, Category = "Pickup", meta = (EditCondition = "!bUseDataDrivenConfig", EditConditionHides, ClampMin = "0.0"))
	float DestroyDelay = 0.0f;

#pragma endregion Pickup Settings

#pragma region Backpack Storage

	UPROPERTY(VisibleAnywhere, Category = "Item|Backpack")
	TArray<FItemData> StoredInventoryItems;

#pragma endregion Backpack Storage

	virtual void BeginPlay() override;
	virtual void LoadConfigFromINI() override;
	bool HandleBackpackPickup(APawn* Picker);
	bool HandleInventoryPickup(APawn* Picker);

public:

	ATFPickupableActor();

#pragma region Interactable Interface Override

	virtual bool Interact(APawn* InstigatorPawn) override;
	virtual FInteractionData GetInteractionData(APawn* InstigatorPawn) const override;

#pragma endregion Interactable Interface Override

#pragma region Pickupable Interface Implementation

	virtual bool OnPickup(APawn* Picker) override;
	virtual FItemData GetItemData() const override;
	virtual bool CanPickup(APawn* Picker) const override;
	virtual void OnPickupFailed(APawn* Picker, const FText& Reason) override;
	virtual bool ShouldDestroyOnPickup() const override;

#pragma endregion Pickupable Interface Implementation

#pragma region Events

	virtual void OnItemPickedUp(APawn* PickerPawn) {}
	virtual void OnItemPickupFailed(APawn* PickerPawn, const FText& Reason) {}

#pragma endregion Events

#pragma region Accessors

	FItemData GetItemInfo() const { return ItemData; }
	void SetItemData(const FItemData& NewItemData);
	void SetStoredInventoryItems(const TArray<FItemData>& Items) { StoredInventoryItems = Items; }
	const TArray<FItemData>& GetStoredInventoryItems() const { return StoredInventoryItems; }

	FORCEINLINE EItemType GetItemType() const { return ItemData.ItemType; }

#pragma endregion Accessors
};
