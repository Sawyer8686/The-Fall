// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TFPlayerController.generated.h"

class UTFLockProgressManager;

/**
 * TF Player Controller
 * Manages player input and UI components
 */
UCLASS()
class TF_API ATFPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** Lock progress manager component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UTFLockProgressManager* LockProgressManager;

	virtual void BeginPlay() override;

public:

	ATFPlayerController();

	/** Get the lock progress manager */
	UTFLockProgressManager* GetLockProgressManager() const { return LockProgressManager; }
};
