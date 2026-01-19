// Fill out your copyright notice in the Description page of Project Settings.

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

	UPROPERTY(EditAnywhere, Category = "Item")
	FItemData ItemData;

#pragma endregion Item Data

#pragma region Pickup Settings

	UPROPERTY(EditAnywhere, Category = "Pickup")
	bool bDestroyOnPickup = true;

	UPROPERTY(EditAnywhere, Category = "Pickup", meta = (ClampMin = "0.0"))
	float DestroyDelay = 0.0f;

#pragma endregion Pickup Settings

	virtual void BeginPlay() override;

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
	void SetQuantity(int32 NewQuantity);

#pragma endregion Accessors
};
