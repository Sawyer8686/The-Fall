// Fill out your copyright notice in the Description page of Project Settings.

#include "TFKeyActor.h"
#include "TFKeyHolderInterface.h"

ATFKeyActor::ATFKeyActor()
{
    InteractionText = NSLOCTEXT("TFKey", "PickUpKey", "Pick Up Key");
    InteractionDuration = 0.0f;
    bDestroyOnPickup = true;
}

bool ATFKeyActor::OnPickup_Implementation(APawn* Picker)
{
    if (!Picker || KeyID.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("TFKeyActor: Invalid pickup - no character or KeyID"));
        return false;
    }

    if (!Picker->Implements<UTFKeyHolderInterface>())
    {
        UE_LOG(LogTemp, Warning, TEXT("TFKeyActor: Picker does not implement ITFKeyHolderInterface"));
        return false;
    }

    ITFKeyHolderInterface::Execute_AddKey(Picker, KeyID);

    UE_LOG(LogTemp, Log, TEXT("TFKeyActor: Collected key '%s' (%s)"), *KeyID.ToString(), *KeyName.ToString());

    OnKeyCollected(Picker);

    return true;
}

FInteractionData ATFKeyActor::GetInteractionData_Implementation(APawn* InstigatorPawn) const
{
    FInteractionData Data = Super::GetInteractionData_Implementation(InstigatorPawn);

    if (!KeyName.IsEmpty())
    {
        Data.InteractionText = FText::Format(
            NSLOCTEXT("TFKey", "PickUpNamedKey", "Pick Up {0}"),
            KeyName
        );
    }
    else
    {
        Data.InteractionText = NSLOCTEXT("TFKey", "PickUpKey", "Pick Up Key");
    }

    if (!KeyDescription.IsEmpty())
    {
        Data.SecondaryText = KeyDescription;
    }

    return Data;
}

void ATFKeyActor::SetKeyID(FName NewKeyID)
{
    KeyID = NewKeyID;
}
