// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TFInteractableActor.h"
#include "TFPickupableInterface.h"
#include "TFPickupableActor.generated.h"

class UAudioComponent;

UCLASS()
class TFWORLDACTORS_API ATFPickupableActor : public ATFInteractableActor, public ITFPickupableInterface
{
	GENERATED_BODY()

protected:

#pragma region Data-Driven Config

	/** Unique identifier for this item. Used to load configuration from ItemConfig.ini */
	UPROPERTY(EditAnywhere, Category = "Item|Config")
	FName ItemID = NAME_None;

#pragma endregion Data-Driven Config

#pragma region Components

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UAudioComponent* AudioComponent;

#pragma endregion Components

#pragma region Item Data

	/** Item data - populated from INI if bUseDataDrivenConfig is true */
	UPROPERTY(EditAnywhere, Category = "Item", meta = (EditCondition = "!bUseDataDrivenConfig", EditConditionHides))
	FItemData ItemData;

#pragma endregion Item Data

#pragma region Pickup Settings

	UPROPERTY(EditAnywhere, Category = "Pickup", meta = (EditCondition = "!bUseDataDrivenConfig", EditConditionHides))
	bool bDestroyOnPickup = true;

	UPROPERTY(EditAnywhere, Category = "Pickup", meta = (EditCondition = "!bUseDataDrivenConfig", EditConditionHides, ClampMin = "0.0"))
	float DestroyDelay = 0.0f;

#pragma endregion Pickup Settings

	virtual void BeginPlay() override;

	/** Load item-specific configuration from ItemConfig.ini */
	virtual void LoadConfigFromINI() override;

	/** Play pickup sound effect */
	void PlayPickupSound();

	/** Handle key-specific pickup logic */
	bool HandleKeyPickup(APawn* Picker);

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

	/** Called when a key item is collected (only when ItemType == Key) */
	virtual void OnKeyCollected(APawn* CollectorPawn) {}

#pragma endregion Events

#pragma region Accessors

	FItemData GetItemInfo() const { return ItemData; }
	void SetItemData(const FItemData& NewItemData);
	void SetQuantity(int32 NewQuantity);

	FORCEINLINE bool IsKey() const { return ItemData.ItemType == EItemType::Key; }
	FORCEINLINE FName GetKeyID() const { return ItemData.KeyID; }
	FORCEINLINE EItemType GetItemType() const { return ItemData.ItemType; }

#pragma endregion Accessors
};
