// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Log Categories
DECLARE_LOG_CATEGORY_EXTERN(LogTFInteraction, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogTFDoor, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogTFItem, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogTFCharacter, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogTFStats, Log, All);

// Stat Names
namespace TFStatNames
{
	const FName Hunger = FName(TEXT("Hunger"));
	const FName Thirst = FName(TEXT("Thirst"));
}
