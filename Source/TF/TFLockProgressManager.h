// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TFLockProgressManager.generated.h"

class UTFLockProgressWidget;
class ATFPlayerController;

/**
 * Manager component that handles lock progress widget creation and binding
 * Attached to the PlayerController
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TF_API UTFLockProgressManager : public UActorComponent
{
	GENERATED_BODY()

protected:

	/** Widget class for lock progress display */
	UPROPERTY(EditDefaultsOnly, Category = "Lock Progress")
	TSubclassOf<UTFLockProgressWidget> LockProgressWidgetClass;

	/** Z-order for the widget (higher = on top) */
	UPROPERTY(EditDefaultsOnly, Category = "Lock Progress")
	int32 WidgetZOrder = 5;

private:

	/** Active lock progress widget instance */
	UPROPERTY()
	UTFLockProgressWidget* LockProgressWidget;

	/** Cached player controller reference */
	UPROPERTY()
	ATFPlayerController* CachedPlayerController;

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Create the widget and add to viewport */
	void CreateWidget();

	/** Bind to player controller delegates */
	void BindToPlayerController();

	/** Unbind from player controller delegates */
	void UnbindFromPlayerController();

	/** Delegate handlers */
	UFUNCTION()
	void HandleLockActionStarted(float Duration, bool bIsUnlocking);

	UFUNCTION()
	void HandleLockActionProgress(float ElapsedTime);

	UFUNCTION()
	void HandleLockActionCompleted();

	UFUNCTION()
	void HandleLockActionCancelled();

	UFUNCTION()
	void HandleLockActionKeyBroken();

public:

	UTFLockProgressManager();

	/** Get the active widget (may be null) */
	UTFLockProgressWidget* GetLockProgressWidget() const { return LockProgressWidget; }
};
