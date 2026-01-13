
// Fill out your copyright notice in the Description page of Project Settings.

#include "TFUnlockedDoorActor.h"

ATFUnlockedDoorActor::ATFUnlockedDoorActor()
{
	// Set default properties for unlocked door
	bCanOpenFromBothSides = true;
	bAutoClose = false;

	// Interaction text
	InteractionText = FText::FromString("Open Door");
}

bool ATFUnlockedDoorActor::IsDoorLocked_Implementation() const
{
	// Unlocked doors are never locked
	return false;
}
