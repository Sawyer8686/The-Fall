// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TFPickupableActor.h"
#include "TFKeyActor.generated.h"

UCLASS(Blueprintable)
class TFWORLDACTORS_API ATFKeyActor : public ATFPickupableActor
{
    GENERATED_BODY()

protected:

#pragma region Key Settings

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key")
    FName KeyID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key")
    FText KeyName = FText::FromString("Key");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key")
    FText KeyDescription = FText::GetEmpty();

#pragma endregion Key Settings

public:

    ATFKeyActor();

#pragma region Pickupable Interface Override

    virtual bool OnPickup_Implementation(APawn* Picker) override;
    virtual FInteractionData GetInteractionData_Implementation(APawn* InstigatorPawn) const override;

#pragma endregion Pickupable Interface Override

#pragma region Blueprint Events

    UFUNCTION(BlueprintImplementableEvent, Category = "Key")
    void OnKeyCollected(APawn* CollectorPawn);

#pragma endregion Blueprint Events

#pragma region Accessors

    UFUNCTION(BlueprintPure, Category = "Key")
    FORCEINLINE FName GetKeyID() const { return KeyID; }

    UFUNCTION(BlueprintPure, Category = "Key")
    FORCEINLINE FText GetKeyName() const { return KeyName; }

    UFUNCTION(BlueprintCallable, Category = "Key")
    void SetKeyID(FName NewKeyID);

#pragma endregion Accessors
};
