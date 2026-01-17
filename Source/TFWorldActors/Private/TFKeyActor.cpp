// Fill out your copyright notice in the Description page of Project Settings.

#include "TFKeyActor.h"
#include "TFPlayerCharacter.h"

ATFKeyActor::ATFKeyActor()
{
    InteractionText = FText::FromString("Pick Up Key");
    InteractionDuration = 0.0f;
    bDestroyOnPickup = true;
}

bool ATFKeyActor::OnPickup_Implementation(APawn* Picker)
{
    ATFPlayerCharacter* PlayerCharacter = Cast<ATFPlayerCharacter>(Picker);

    if (!PlayerCharacter || KeyID.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("TFKeyActor: Invalid pickup - no character or KeyID"));
        return false;
    }

    PlayerCharacter->AddKey(KeyID);

    UE_LOG(LogTemp, Log, TEXT("TFKeyActor: Collected key '%s' (%s)"), *KeyID.ToString(), *KeyName.ToString());

    OnKeyCollected(PlayerCharacter);

    return true;
}

FInteractionData ATFKeyActor::GetInteractionData_Implementation(APawn* Instigator) const
{
    FInteractionData Data = Super::GetInteractionData_Implementation(Instigator);

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

    if (!KeyDescription.IsEmpty())
    {
        Data.SecondaryText = KeyDescription;
    }

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
