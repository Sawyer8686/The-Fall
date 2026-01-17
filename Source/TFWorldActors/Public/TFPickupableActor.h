// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TFInteractableActor.h"
#include "TFPickupableInterface.h"
#include "TFPickupableActor.generated.h"

UCLASS(Blueprintable)
class TFWORLDACTORS_API ATFPickupableActor : public ATFInteractableActor, public ITFPickupableInterface
{
	GENERATED_BODY()

protected:

#pragma region Item Data

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FItemData ItemData;

#pragma endregion Item Data

#pragma region Pickup Settings

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	bool bDestroyOnPickup = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", meta = (ClampMin = "0.0"))
	float DestroyDelay = 0.0f;


#pragma endregion Pickup Settings

	virtual void BeginPlay() override;

public:

	ATFPickupableActor();

#pragma region Interactable Interface Override

	virtual bool Interact_Implementation(APawn* InstigatorPawn) override;
	virtual FInteractionData GetInteractionData_Implementation(APawn* InstigatorPawn) const override;

#pragma endregion Interactable Interface Override

#pragma region Pickupable Interface Implementation

	virtual bool OnPickup_Implementation(APawn* Picker) override;
	virtual FItemData GetItemData_Implementation() const override;
	virtual bool CanPickup_Implementation(APawn* Picker) const override;
	virtual void OnPickupFailed_Implementation(APawn* Picker, const FText& Reason) override;
	virtual bool ShouldDestroyOnPickup_Implementation() const override;

#pragma endregion Pickupable Interface Implementation

#pragma region Blueprint Events

	UFUNCTION(BlueprintImplementableEvent, Category = "Pickup")
	void OnItemPickedUp(APawn* PickerPawn);

	UFUNCTION(BlueprintImplementableEvent, Category = "Pickup")
	void OnItemPickupFailed(APawn* PickerPawn, const FText& Reason);

#pragma endregion Blueprint Events

#pragma region Accessors

	UFUNCTION(BlueprintPure, Category = "Item")
	FItemData GetItemInfo() const { return ItemData; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetItemData(const FItemData& NewItemData);

	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetQuantity(int32 NewQuantity);

#pragma endregion Accessors
};
