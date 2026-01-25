// Copyright TF Project. All Rights Reserved.

#include "TFLockProgressManager.h"
#include "TFLockProgressWidget.h"
#include "TFPlayerController.h"
#include "Blueprint/UserWidget.h"

UTFLockProgressManager::UTFLockProgressManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTFLockProgressManager::BeginPlay()
{
	Super::BeginPlay();

	CreateWidget();
	BindToPlayerController();
}

void UTFLockProgressManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindFromPlayerController();

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

	ATFPlayerController* PC = Cast<ATFPlayerController>(GetOwner());
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

void UTFLockProgressManager::BindToPlayerController()
{
	CachedPlayerController = Cast<ATFPlayerController>(GetOwner());

	if (!CachedPlayerController)
	{
		return;
	}

	CachedPlayerController->OnLockActionStarted.AddDynamic(this, &UTFLockProgressManager::HandleLockActionStarted);
	CachedPlayerController->OnLockActionProgress.AddDynamic(this, &UTFLockProgressManager::HandleLockActionProgress);
	CachedPlayerController->OnLockActionCompleted.AddDynamic(this, &UTFLockProgressManager::HandleLockActionCompleted);
	CachedPlayerController->OnLockActionCancelled.AddDynamic(this, &UTFLockProgressManager::HandleLockActionCancelled);
}

void UTFLockProgressManager::UnbindFromPlayerController()
{
	if (!CachedPlayerController)
	{
		return;
	}

	CachedPlayerController->OnLockActionStarted.RemoveDynamic(this, &UTFLockProgressManager::HandleLockActionStarted);
	CachedPlayerController->OnLockActionProgress.RemoveDynamic(this, &UTFLockProgressManager::HandleLockActionProgress);
	CachedPlayerController->OnLockActionCompleted.RemoveDynamic(this, &UTFLockProgressManager::HandleLockActionCompleted);
	CachedPlayerController->OnLockActionCancelled.RemoveDynamic(this, &UTFLockProgressManager::HandleLockActionCancelled);

	CachedPlayerController = nullptr;
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
