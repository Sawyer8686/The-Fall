// Copyright TF Project. All Rights Reserved.

#include "TFGameMode.h"
#include "TFPlayerController.h"
#include "TFPlayerCharacter.h"
#include "TFDayNightCycle.h"
#include "Kismet/GameplayStatics.h"

ATFGameMode::ATFGameMode()
{
	PlayerControllerClass = ATFPlayerController::StaticClass();
	DefaultPawnClass = ATFPlayerCharacter::StaticClass();
}

void ATFGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoFindDayNightCycle)
	{
		FindDayNightCycle();
	}
}

void ATFGameMode::FindDayNightCycle()
{
	if (DayNightCycle)
	{
		return;
	}

	DayNightCycle = Cast<ATFDayNightCycle>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ATFDayNightCycle::StaticClass())
	);

	if (DayNightCycle)
	{
		UE_LOG(LogTemp, Log, TEXT("TFGameMode: Found Day/Night Cycle actor"));
	}
}

void ATFGameMode::SetDayNightCycle(ATFDayNightCycle* NewDayNightCycle)
{
	DayNightCycle = NewDayNightCycle;
}
