// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TFLockProgressManager.generated.h"

class UTFLockProgressWidget;
class ATFPlayerCharacter;

/**
 * Manager component that handles lock progress widget creation and binding
 * Add this component to the PlayerController or use it in a HUD class
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WIDGETS_API UTFLockProgressManager : public UActorComponent
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

	/** Cached player character reference */
	UPROPERTY()
	ATFPlayerCharacter* CachedPlayerCharacter;

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Create the widget and add to viewport */
	void CreateWidget();

	/** Bind to player character delegates */
	void BindToPlayerCharacter();

	/** Unbind from player character delegates */
	void UnbindFromPlayerCharacter();

	/** Delegate handlers */
	void HandleLockActionStarted(float Duration, bool bIsUnlocking);
	void HandleLockActionProgress(float ElapsedTime);
	void HandleLockActionCompleted();
	void HandleLockActionCancelled();

public:

	UTFLockProgressManager();

	/** Get the active widget (may be null) */
	UTFLockProgressWidget* GetLockProgressWidget() const { return LockProgressWidget; }
};
