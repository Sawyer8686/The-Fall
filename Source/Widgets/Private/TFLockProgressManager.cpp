// Copyright TF Project. All Rights Reserved.

#include "TFLockProgressManager.h"
#include "TFLockProgressWidget.h"
#include "TFPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

UTFLockProgressManager::UTFLockProgressManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTFLockProgressManager::BeginPlay()
{
	Super::BeginPlay();

	CreateWidget();
	BindToPlayerCharacter();
}

void UTFLockProgressManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindFromPlayerCharacter();

	if (LockProgressWidget)
	{
		LockProgressWidget->RemoveFromParent();
		LockProgressWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void UTFLockProgressManager::CreateWidget()
{
	if (!LockProgressWidgetClass)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC)
	{
		PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	}

	if (!PC)
	{
		return;
	}

	LockProgressWidget = ::CreateWidget<UTFLockProgressWidget>(PC, LockProgressWidgetClass);
	if (LockProgressWidget)
	{
		LockProgressWidget->AddToViewport(WidgetZOrder);
	}
}

void UTFLockProgressManager::BindToPlayerCharacter()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	CachedPlayerCharacter = Cast<ATFPlayerCharacter>(PlayerPawn);

	if (!CachedPlayerCharacter)
	{
		return;
	}

	CachedPlayerCharacter->OnLockActionStarted.AddUObject(this, &UTFLockProgressManager::HandleLockActionStarted);
	CachedPlayerCharacter->OnLockActionProgress.AddUObject(this, &UTFLockProgressManager::HandleLockActionProgress);
	CachedPlayerCharacter->OnLockActionCompleted.AddUObject(this, &UTFLockProgressManager::HandleLockActionCompleted);
	CachedPlayerCharacter->OnLockActionCancelled.AddUObject(this, &UTFLockProgressManager::HandleLockActionCancelled);
}

void UTFLockProgressManager::UnbindFromPlayerCharacter()
{
	if (!CachedPlayerCharacter)
	{
		return;
	}

	CachedPlayerCharacter->OnLockActionStarted.RemoveAll(this);
	CachedPlayerCharacter->OnLockActionProgress.RemoveAll(this);
	CachedPlayerCharacter->OnLockActionCompleted.RemoveAll(this);
	CachedPlayerCharacter->OnLockActionCancelled.RemoveAll(this);

	CachedPlayerCharacter = nullptr;
}

void UTFLockProgressManager::HandleLockActionStarted(float Duration, bool bIsUnlocking)
{
	if (LockProgressWidget)
	{
		LockProgressWidget->StartProgress(Duration, bIsUnlocking);
	}
}

void UTFLockProgressManager::HandleLockActionProgress(float ElapsedTime)
{
	if (LockProgressWidget)
	{
		LockProgressWidget->UpdateProgress(ElapsedTime);
	}
}

void UTFLockProgressManager::HandleLockActionCompleted()
{
	if (LockProgressWidget)
	{
		LockProgressWidget->CompleteProgress();
	}
}

void UTFLockProgressManager::HandleLockActionCancelled()
{
	if (LockProgressWidget)
	{
		LockProgressWidget->CancelProgress();
	}
}
