// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "TFPlayerController.generated.h"

class UTFLockProgressManager;
class UInputMappingContext;
class UInputAction;
class ATFPlayerCharacter;
class UTFStatsWidget;
class UTFStaminaWidget;
class UTFDayNightWidget;
class UTFInventoryWidget;
class UTFBackpackIndicatorWidget;
class UTFBackpackConfirmWidget;
class UTFContainerWidget;
class ATFBaseContainerActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryToggled, bool, bIsOpen);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBackpackEquipRequested, int32, Slots, float, WeightLimit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLockActionStarted, float, Duration, bool, bIsUnlocking);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockActionProgress, float, ElapsedTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLockActionCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLockActionCancelled);

/**
 * TF Player Controller
 * Manages player input, UI state, and widget components
 */
UCLASS()
class TF_API ATFPlayerController : public APlayerController
{
	GENERATED_BODY()

#pragma region Components

protected:

	/** Lock progress manager component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UTFLockProgressManager* LockProgressManager;

#pragma endregion Components

#pragma region Input

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* SprintAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* SneakAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* InteractAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* LockAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* InventoryAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* DropBackpackAction;

#pragma endregion Input

#pragma region Widget Classes

protected:

	UPROPERTY(EditDefaultsOnly, Category = "UI|Widgets")
	TSubclassOf<UTFStatsWidget> StatsWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Widgets")
	TSubclassOf<UTFStaminaWidget> StaminaWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Widgets")
	TSubclassOf<UTFDayNightWidget> DayNightWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Widgets")
	TSubclassOf<UTFInventoryWidget> InventoryWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Widgets")
	TSubclassOf<UTFBackpackIndicatorWidget> BackpackIndicatorWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Widgets")
	TSubclassOf<UTFBackpackConfirmWidget> BackpackConfirmWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Widgets")
	TSubclassOf<UTFContainerWidget> ContainerWidgetClass;

#pragma endregion Widget Classes

#pragma region Widget Instances

private:

	UPROPERTY()
	UTFStatsWidget* StatsWidget;

	UPROPERTY()
	UTFStaminaWidget* StaminaWidget;

	UPROPERTY()
	UTFDayNightWidget* DayNightWidget;

	UPROPERTY()
	UTFInventoryWidget* InventoryWidget;

	UPROPERTY()
	UTFBackpackIndicatorWidget* BackpackIndicatorWidget;

	UPROPERTY()
	UTFBackpackConfirmWidget* BackpackConfirmWidget;

	UPROPERTY()
	UTFContainerWidget* ContainerWidget;

#pragma endregion Widget Instances

#pragma region UI State

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	bool bInventoryOpen = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	bool bConfirmDialogOpen = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	bool bContainerOpen = false;

#pragma endregion UI State

#pragma region Lock Action State

private:

	FTimerHandle LockHoldTimerHandle;
	FTimerHandle LockProgressTimerHandle;
	TWeakObjectPtr<AActor> LockTarget;
	float LockActionDuration = 0.0f;
	float LockActionElapsedTime = 0.0f;
	bool bIsUnlockingAction = true;

#pragma endregion Lock Action State

#pragma region Cached References

private:

	UPROPERTY()
	TWeakObjectPtr<ATFPlayerCharacter> CachedPlayerCharacter;

	UPROPERTY()
	TWeakObjectPtr<ATFBaseContainerActor> CurrentContainer;

#pragma endregion Cached References

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void SetupInputComponent() override;

	/** Bind to character delegates */
	void BindToCharacter(ATFPlayerCharacter* InCharacter);

	/** Unbind from character delegates */
	void UnbindFromCharacter(ATFPlayerCharacter* InCharacter);

	/** Handle backpack equip request from character */
	void HandleBackpackEquipRequested(int32 Slots, float WeightLimit);

#pragma region Widget Management

	/** Create all HUD widgets */
	void CreateHUDWidgets();

	/** Destroy all HUD widgets */
	void DestroyHUDWidgets();

	/** Initialize widget bindings to character components */
	void InitializeWidgetBindings();

#pragma endregion Widget Management

#pragma region Input Handlers

	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleJumpStarted();
	void HandleJumpCompleted();
	void HandleSprintStarted();
	void HandleSprintCompleted();
	void HandleSneakStarted();
	void HandleSneakCompleted();
	void HandleInteract();
	void HandleLockStarted();
	void HandleLockCompleted();
	void HandleInventory();
	void HandleDropBackpack();

#pragma endregion Input Handlers

#pragma region Lock Action

	void UpdateLockProgress();
	void CompleteLockAction();

#pragma endregion Lock Action

public:

	ATFPlayerController();

#pragma region Accessors

	/** Get the lock progress manager */
	UFUNCTION(BlueprintCallable, Category = "Components")
	UTFLockProgressManager* GetLockProgressManager() const { return LockProgressManager; }

	/** Check if inventory is open */
	UFUNCTION(BlueprintCallable, Category = "UI")
	bool IsInventoryOpen() const { return bInventoryOpen; }

	/** Check if confirm dialog is open */
	UFUNCTION(BlueprintCallable, Category = "UI")
	bool IsConfirmDialogOpen() const { return bConfirmDialogOpen; }

	/** Check if container is open */
	UFUNCTION(BlueprintCallable, Category = "UI")
	bool IsContainerOpen() const { return bContainerOpen; }

	/** Check if any UI is blocking gameplay input */
	UFUNCTION(BlueprintCallable, Category = "UI")
	bool IsUIBlockingInput() const { return bInventoryOpen || bConfirmDialogOpen || bContainerOpen; }

	/** Get cached player character */
	UFUNCTION(BlueprintCallable, Category = "Character")
	ATFPlayerCharacter* GetTFPlayerCharacter() const;

	/** Get widget instances */
	UFUNCTION(BlueprintCallable, Category = "UI")
	UTFStatsWidget* GetStatsWidget() const { return StatsWidget; }

	UFUNCTION(BlueprintCallable, Category = "UI")
	UTFStaminaWidget* GetStaminaWidget() const { return StaminaWidget; }

	UFUNCTION(BlueprintCallable, Category = "UI")
	UTFDayNightWidget* GetDayNightWidget() const { return DayNightWidget; }

	UFUNCTION(BlueprintCallable, Category = "UI")
	UTFInventoryWidget* GetInventoryWidget() const { return InventoryWidget; }

	UFUNCTION(BlueprintCallable, Category = "UI")
	UTFBackpackIndicatorWidget* GetBackpackIndicatorWidget() const { return BackpackIndicatorWidget; }

	UFUNCTION(BlueprintCallable, Category = "UI")
	UTFContainerWidget* GetContainerWidget() const { return ContainerWidget; }

#pragma endregion Accessors

#pragma region UI Control

	/** Set UI input mode with cursor visibility */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetUIInputMode(bool bShowCursor);

	/** Toggle inventory open/close */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleInventory();

	/** Open backpack equip confirmation dialog */
	void OpenBackpackConfirmDialog(int32 Slots, float WeightLimit);

	/** Close backpack equip confirmation dialog */
	void CloseBackpackConfirmDialog(bool bConfirmed);

	/** Open container UI */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void OpenContainer(ATFBaseContainerActor* Container);

	/** Close container UI */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void CloseContainer();

#pragma endregion UI Control

#pragma region Delegates

public:

	/** Called when inventory is toggled */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnInventoryToggled OnInventoryToggled;

	/** Called when backpack equip is requested */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBackpackEquipRequested OnBackpackEquipRequested;

	/** Called when lock/unlock action starts */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLockActionStarted OnLockActionStarted;

	/** Called every tick during lock/unlock action */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLockActionProgress OnLockActionProgress;

	/** Called when lock/unlock action completes */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLockActionCompleted OnLockActionCompleted;

	/** Called when lock/unlock action is cancelled */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLockActionCancelled OnLockActionCancelled;

#pragma endregion Delegates
};
