// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/TFKeyActor.h"
#include "TFPlayerCharacter.h"

ATFKeyActor::ATFKeyActor()
{
    // Set default interaction text for keys
    InteractionText = FText::FromString("Pick Up Key");

    // Keys are instant pickup
    InteractionDuration = 0.0f;

    // Keys should be destroyed on pickup
    bDestroyOnPickup = true;
}

bool ATFKeyActor::OnPickup_Implementation(ATFPlayerCharacter* PickerCharacter)
{
    // Validate
    if (!PickerCharacter || KeyID.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("TFKeyActor: Invalid pickup - no character or KeyID"));
        return false;
    }

    // Add key to player's collection
    PickerCharacter->AddKey(KeyID);

    // Log for debugging
    UE_LOG(LogTemp, Log, TEXT("TFKeyActor: Collected key '%s' (%s)"), *KeyID.ToString(), *KeyName.ToString());

    // Call blueprint event
    OnKeyCollected(PickerCharacter);

    return true;
}

FInteractionData ATFKeyActor::GetInteractionData_Implementation(ATFPlayerCharacter* InstigatorCharacter) const
{
    FInteractionData Data = Super::GetInteractionData_Implementation(InstigatorCharacter);

    // Override with key-specific text
    if (!KeyName.IsEmpty())
    {
        Data.InteractionText = FText::Format(
            FText::FromString("Pick Up {0}"),
            KeyName
        );
    }
    else
    {
        Data.InteractionText = FText::FromString("Pick Up Key");
    }

    // Show key description as secondary text if available
    if (!KeyDescription.IsEmpty())
    {
        Data.SecondaryText = KeyDescription;
    }

    // Use item icon if set
    if (ItemData.ItemIcon)
    {
        Data.InteractionIcon = ItemData.ItemIcon;
    }

    return Data;
}

void ATFKeyActor::SetKeyID(FName NewKeyID)
{
    KeyID = NewKeyID;
}
