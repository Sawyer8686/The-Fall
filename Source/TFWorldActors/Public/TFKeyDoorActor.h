// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TFBaseDoorActor.h"
#include "TFKeyDoorActor.generated.h"

/**
 * Key Door Actor
 * A door that requires a specific key to unlock
 * Can be locked/unlocked by players who have the required key
 */
UCLASS(Blueprintable)
class TFWORLDACTORS_API ATFKeyDoorActor : public ATFBaseDoorActor
{
    GENERATED_BODY()

protected:

#pragma region Key Settings

    /** Key ID required to unlock/lock this door */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Key")
    FName RequiredKeyID = NAME_None;

    /** Display name of the required key (for UI) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Key")
    FText RequiredKeyName = FText::FromString("Key");

    /** Is the door currently in a locked state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Key")
    bool bIsLocked = true;

    /** Can this door be re-locked after unlocking */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Key")
    bool bCanRelock = true;

#pragma endregion Key Settings

#pragma region Audio

    /** Sound played when door is unlocked */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
    USoundBase* DoorUnlockSound;

    /** Sound played when door is locked */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
    USoundBase* DoorLockSound;

#pragma endregion Audio

public:

    ATFKeyDoorActor();

#pragma region Door Interface Override

    /** Override: Check if door is currently locked */
    virtual bool IsDoorLocked_Implementation() const override;

    /** Override: Attempt to unlock the door with a key */
    virtual bool UnlockDoor_Implementation(ATFPlayerCharacter* UnlockingCharacter) override;

    /** Override: Custom interaction handling for key doors */
    virtual bool Interact_Implementation(ATFPlayerCharacter* InstigatorCharacter) override;

    /** Override: Get interaction data based on lock state and key possession */
    virtual FInteractionData GetInteractionData_Implementation(ATFPlayerCharacter* InstigatorCharacter) const override;

#pragma endregion Door Interface Override

#pragma region Lock Control

    /**
     * Lock the door (requires player to have the key)
     * @param LockingCharacter - Character attempting to lock
     * @return True if door was locked
     */
    UFUNCTION(BlueprintCallable, Category = "Door|Key")
    virtual bool LockDoor(ATFPlayerCharacter* LockingCharacter);

    /**
     * Force set lock state (ignores key requirement - for puzzles/triggers)
     * @param bNewLockState - New lock state
     */
    UFUNCTION(BlueprintCallable, Category = "Door|Key")
    void SetLockedState(bool bNewLockState);

#pragma endregion Lock Control

#pragma region Blueprint Events

    /** Called when door is unlocked with a key */
    UFUNCTION(BlueprintImplementableEvent, Category = "Door|Key")
    void OnDoorUnlocked(ATFPlayerCharacter* UnlockingCharacter);

    /** Called when door is locked with a key */
    UFUNCTION(BlueprintImplementableEvent, Category = "Door|Key")
    void OnDoorRelocked(ATFPlayerCharacter* LockingCharacter);

    /** Called when player tries to open without key */
    UFUNCTION(BlueprintImplementableEvent, Category = "Door|Key")
    void OnKeyRequired(ATFPlayerCharacter* AttemptingCharacter);

#pragma endregion Blueprint Events

#pragma region Queries

    /** Get the required key ID */
    UFUNCTION(BlueprintPure, Category = "Door|Key")
    FORCEINLINE FName GetRequiredKeyID() const { return RequiredKeyID; }

    /** Get the required key display name */
    UFUNCTION(BlueprintPure, Category = "Door|Key")
    FORCEINLINE FText GetRequiredKeyName() const { return RequiredKeyName; }

    /** Check if a character has the required key */
    UFUNCTION(BlueprintPure, Category = "Door|Key")
    bool CharacterHasKey(ATFPlayerCharacter* Character) const;

    /** Check if door is in locked state */
    UFUNCTION(BlueprintPure, Category = "Door|Key")
    FORCEINLINE bool IsLocked() const { return bIsLocked; }

#pragma endregion Queries
};
