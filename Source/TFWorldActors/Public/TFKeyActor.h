// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TFPickupableActor.h"
#include "TFKeyActor.generated.h"

UCLASS()
class TFWORLDACTORS_API ATFKeyActor : public ATFPickupableActor
{
    GENERATED_BODY()

protected:

#pragma region Key Settings

    UPROPERTY(EditAnywhere, Category = "Key")
    FName KeyID = NAME_None;

    UPROPERTY(EditAnywhere, Category = "Key")
    FText KeyName = FText::FromString("Key");

    UPROPERTY(EditAnywhere, Category = "Key")
    FText KeyDescription = FText::GetEmpty();

#pragma endregion Key Settings

public:

    ATFKeyActor();

#pragma region Pickupable Interface Override

    virtual bool OnPickup(APawn* Picker) override;
    virtual FInteractionData GetInteractionData(APawn* InstigatorPawn) const override;

#pragma endregion Pickupable Interface Override

#pragma region Events

    virtual void OnKeyCollected(APawn* CollectorPawn) {}

#pragma endregion Events

#pragma region Accessors

    FORCEINLINE FName GetKeyID() const { return KeyID; }
    FORCEINLINE FText GetKeyName() const { return KeyName; }
    void SetKeyID(FName NewKeyID);

#pragma endregion Accessors
};
