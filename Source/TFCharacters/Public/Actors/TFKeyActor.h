// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/TFPickupableActor.h"
#include "TFKeyActor.generated.h"

/**
 * Key Actor
 * A pickupable key that can be used to unlock specific doors
 * When picked up, adds the key to the player's key collection
 */
UCLASS(Blueprintable)
class TFCHARACTERS_API ATFKeyActor : public ATFPickupableActor
{
    GENERATED_BODY()

protected:

#pragma region Key Settings

    /** Unique identifier for this key type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key")
    FName KeyID = NAME_None;

    /** Display name of the key (shown in UI) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key")
    FText KeyName = FText::FromString("Key");

    /** Description of what this key unlocks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key")
    FText KeyDescription = FText::GetEmpty();

#pragma endregion Key Settings

public:

    ATFKeyActor();

#pragma region Pickupable Interface Override

    /** Override to add key to player's collection */
    virtual bool OnPickup_Implementation(ATFPlayerCharacter* PickerCharacter) override;

    /** Override to show key-specific interaction data */
    virtual FInteractionData GetInteractionData_Implementation(ATFPlayerCharacter* InstigatorCharacter) const override;

#pragma endregion Pickupable Interface Override

#pragma region Blueprint Events

    /** Called when key is added to player's collection */
    UFUNCTION(BlueprintImplementableEvent, Category = "Key")
    void OnKeyCollected(ATFPlayerCharacter* CollectorCharacter);

#pragma endregion Blueprint Events

#pragma region Accessors

    /** Get the key ID */
    UFUNCTION(BlueprintPure, Category = "Key")
    FORCEINLINE FName GetKeyID() const { return KeyID; }

    /** Get the key display name */
    UFUNCTION(BlueprintPure, Category = "Key")
    FORCEINLINE FText GetKeyName() const { return KeyName; }

    /** Set the key ID */
    UFUNCTION(BlueprintCallable, Category = "Key")
    void SetKeyID(FName NewKeyID);

#pragma endregion Accessors
};
