// Fill out your copyright notice in the Description page of Project Settings.

#include "TFKeyActor.h"
#include "TFKeyHolderInterface.h"

ATFKeyActor::ATFKeyActor()
{
    InteractionDuration = 0.0f;
    bDestroyOnPickup = true;
}

bool ATFKeyActor::OnPickup(APawn* Picker)
{
    if (!Picker || KeyID.IsNone())
    {
        return false;
    }

    ITFKeyHolderInterface* KeyHolder = Cast<ITFKeyHolderInterface>(Picker);
    if (!KeyHolder)
    {
        return false;
    }

    KeyHolder->AddKey(KeyID);

    OnKeyCollected(Picker);

    return true;
}

FInteractionData ATFKeyActor::GetInteractionData(APawn* InstigatorPawn) const
{
    FInteractionData Data = Super::GetInteractionData(InstigatorPawn);
    return Data;
}

void ATFKeyActor::SetKeyID(FName NewKeyID)
{
    KeyID = NewKeyID;
}
