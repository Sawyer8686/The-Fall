// Copyright TF Project. All Rights Reserved.

#include "TFGameMode.h"
#include "TFPlayerController.h"
#include "TFPlayerCharacter.h"

ATFGameMode::ATFGameMode()
{
	PlayerControllerClass = ATFPlayerController::StaticClass();
	DefaultPawnClass = ATFPlayerCharacter::StaticClass();
}
