// Copyright TF Project. All Rights Reserved.

#include "TFPlayerController.h"
#include "TFLockProgressManager.h"

ATFPlayerController::ATFPlayerController()
{
	LockProgressManager = CreateDefaultSubobject<UTFLockProgressManager>(TEXT("LockProgressManager"));
}

void ATFPlayerController::BeginPlay()
{
	Super::BeginPlay();
}
