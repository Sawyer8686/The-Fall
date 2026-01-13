// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TFBaseDoorActor.h"
#include "TFUnlockedDoorActor.generated.h"

/**
 * Unlocked Door Actor
 * A simple door that can be opened and closed freely by the player
 * No keys, locks, or special requirements
 */
UCLASS(Blueprintable)
class TFWORLDACTORS_API ATFUnlockedDoorActor : public ATFBaseDoorActor
{
	GENERATED_BODY()

public:

	ATFUnlockedDoorActor();

	/**
	 * Unlocked doors are never locked
	 * @return Always false
	 */
	virtual bool IsDoorLocked_Implementation() const override;
};
