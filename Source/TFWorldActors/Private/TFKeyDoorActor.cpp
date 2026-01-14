// Fill out your copyright notice in the Description page of Project Settings.

#include "TFKeyDoorActor.h"
#include "TFPlayerCharacter.h"
#include "Components/AudioComponent.h"

ATFKeyDoorActor::ATFKeyDoorActor()
{
    // Set default properties for key door
    bCanOpenFromBothSides = true;
    bAutoClose = false;
    bIsLocked = true;

    // Default interaction text
    InteractionText = FText::FromString("Open Door");
}

bool ATFKeyDoorActor::IsDoorLocked_Implementation() const
{
    return bIsLocked;
}

bool ATFKeyDoorActor::UnlockDoor_Implementation(ATFPlayerCharacter* UnlockingCharacter)
{
    // Check if character has the required key
    if (!CharacterHasKey(UnlockingCharacter))
    {
        return false;
    }

    // Already unlocked
    if (!bIsLocked)
    {
        return false;
    }

    // Unlock the door
    bIsLocked = false;

    // Play unlock sound
    PlayDoorSound(DoorUnlockSound);

    // Call blueprint event
    OnDoorUnlocked(UnlockingCharacter);

    UE_LOG(LogTemp, Log, TEXT("TFKeyDoorActor: Door unlocked with key '%s'"), *RequiredKeyID.ToString());

    return true;
}

bool ATFKeyDoorActor::LockDoor(ATFPlayerCharacter* LockingCharacter)
{
    // Check if relocking is allowed
    if (!bCanRelock)
    {
        return false;
    }

    // Check if character has the required key
    if (!CharacterHasKey(LockingCharacter))
    {
        return false;
    }

    // Already locked
    if (bIsLocked)
    {
        return false;
    }

    // Door must be closed to lock
    if (!IsClosed())
    {
        return false;
    }

    // Lock the door
    bIsLocked = true;

    // Play lock sound
    PlayDoorSound(DoorLockSound);

    // Call blueprint event
    OnDoorRelocked(LockingCharacter);

    UE_LOG(LogTemp, Log, TEXT("TFKeyDoorActor: Door locked with key '%s'"), *RequiredKeyID.ToString());

    return true;
}

void ATFKeyDoorActor::SetLockedState(bool bNewLockState)
{
    bIsLocked = bNewLockState;
}

bool ATFKeyDoorActor::Interact_Implementation(ATFPlayerCharacter* InstigatorCharacter)
{
    if (!InstigatorCharacter)
    {
        return false;
    }

    bool bHasKey = CharacterHasKey(InstigatorCharacter);

    // Case 1: Door is locked
    if (bIsLocked)
    {
        if (bHasKey)
        {
            // Player has key - unlock the door
            return UnlockDoor(InstigatorCharacter);
        }
        else
        {
            // Player doesn't have key - show "locked" feedback
            PlayDoorSound(DoorLockedSound);
            OnKeyRequired(InstigatorCharacter);
            OnDoorLocked(InstigatorCharacter);
            return false;
        }
    }

    // Case 2: Door is unlocked and closed - open it
    if (IsClosed())
    {
        return OpenDoor(InstigatorCharacter);
    }

    // Case 3: Door is unlocked and open - close it
    if (IsOpen())
    {
        return CloseDoor();
    }

    // Door is moving, do nothing
    return false;
}

FInteractionData ATFKeyDoorActor::GetInteractionData_Implementation(ATFPlayerCharacter* InstigatorCharacter) const
{
    FInteractionData Data;
    Data.bCanInteract = true;
    Data.InteractionDuration = 0.0f;

    bool bHasKey = CharacterHasKey(const_cast<ATFPlayerCharacter*>(InstigatorCharacter));

    // Locked door states
    if (bIsLocked)
    {
        if (bHasKey)
        {
            // Player has the key - show unlock option
            Data.InteractionText = FText::FromString("Unlock Door");
            Data.SecondaryText = FText::Format(
                FText::FromString("Using {0}"),
                RequiredKeyName
            );
            Data.bCanInteract = true;
        }
        else
        {
            // Player doesn't have the key - show requirement
            Data.InteractionText = FText::FromString("Locked");
            Data.SecondaryText = FText::Format(
                FText::FromString("Requires {0}"),
                RequiredKeyName
            );
            Data.bCanInteract = false;
        }
    }
    // Unlocked door states
    else
    {
        if (IsClosed())
        {
            Data.InteractionText = FText::FromString("Open Door");
            Data.bCanInteract = true;
        }
        else if (IsOpen())
        {
            Data.InteractionText = FText::FromString("Close Door");
            Data.bCanInteract = true;
        }
        else
        {
            // Door is moving
            Data.InteractionText = FText::FromString("Wait...");
            Data.bCanInteract = false;
        }
    }

    return Data;
}

bool ATFKeyDoorActor::CharacterHasKey(ATFPlayerCharacter* Character) const
{
    if (!Character || RequiredKeyID.IsNone())
    {
        return false;
    }

    return Character->HasKey(RequiredKeyID);
}
