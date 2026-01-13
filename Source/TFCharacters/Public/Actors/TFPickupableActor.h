// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TFInteractableActor.h"
#include "TFPickupableInterface.h"
#include "TFPickupableActor.generated.h"

class URotatingMovementComponent;

/**
 * Base class for pickupable items in the world
 * Items that can be collected and added to inventory
 */
UCLASS(Blueprintable)
class TFCHARACTERS_API ATFPickupableActor : public ATFInteractableActor, public ITFPickupableInterface
{
	GENERATED_BODY()

protected:

#pragma region Item Data

	/** Item data for inventory */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FItemData ItemData;

#pragma endregion Item Data

#pragma region Pickup Settings

	/** Should this actor be destroyed after pickup */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	bool bDestroyOnPickup = true;

	/** Delay before destroying after pickup (for effects) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", meta = (ClampMin = "0.0"))
	float DestroyDelay = 0.0f;

	/** Should play pickup animation/effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Effects")
	bool bPlayPickupEffects = true;

	/** Pickup sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Effects")
	USoundBase* PickupSound = nullptr;

	/** Pickup particle effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Effects")
	UParticleSystem* PickupParticle = nullptr;

#pragma endregion Pickup Settings

#pragma region Visual Settings

	/** Enable rotation animation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Visual")
	bool bEnableRotation = true;

	/** Rotation speed (degrees per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Visual")
	float RotationSpeed = 90.0f;

	/** Enable bobbing animation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Visual")
	bool bEnableBobbing = true;

	/** Bobbing amplitude */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Visual")
	float BobbingAmplitude = 10.0f;

	/** Bobbing frequency */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Visual")
	float BobbingFrequency = 1.0f;

	/** Rotating movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	URotatingMovementComponent* RotatingMovement;

	/** Initial Z location for bobbing */
	float InitialZ = 0.0f;

	/** Bobbing timer */
	float BobbingTimer = 0.0f;

#pragma endregion Visual Settings

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Play pickup effects (sound, particles) */
	virtual void PlayPickupEffects();

	/** Update bobbing animation */
	void UpdateBobbing(float DeltaTime);

public:

	ATFPickupableActor();

#pragma region Interactable Interface Override

	/** Override interact to handle pickup */
	virtual bool Interact_Implementation(ATFPlayerCharacter* InstigatorCharacter) override;

	/** Override interaction data to show pickup info */
	virtual FInteractionData GetInteractionData_Implementation(ATFPlayerCharacter* InstigatorCharacter) const override;

#pragma endregion Interactable Interface Override

#pragma region Pickupable Interface Implementation

	/** Handle pickup */
	virtual bool OnPickup_Implementation(ATFPlayerCharacter* PickerCharacter) override;

	/** Get item data */
	virtual FItemData GetItemData_Implementation() const override;

	/** Check if can pickup */
	virtual bool CanPickup_Implementation(ATFPlayerCharacter* PickerCharacter) const override;

	/** Handle pickup failure */
	virtual void OnPickupFailed_Implementation(ATFPlayerCharacter* PickerCharacter, const FText& Reason) override;

	/** Should destroy on pickup */
	virtual bool ShouldDestroyOnPickup_Implementation() const override;

#pragma endregion Pickupable Interface Implementation

#pragma region Blueprint Events

	/** Called when item is successfully picked up (implement in BP) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Pickup")
	void OnItemPickedUp(ATFPlayerCharacter* PickerCharacter);

	/** Called when pickup fails (implement in BP) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Pickup")
	void OnItemPickupFailed(ATFPlayerCharacter* PickerCharacter, const FText& Reason);

#pragma endregion Blueprint Events

#pragma region Accessors

	/** Get item data */
	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE FItemData GetItemInfo() const { return ItemData; }

	/** Set item data */
	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetItemData(const FItemData& NewItemData);

	/** Set item quantity */
	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetQuantity(int32 NewQuantity);

#pragma endregion Accessors
};
