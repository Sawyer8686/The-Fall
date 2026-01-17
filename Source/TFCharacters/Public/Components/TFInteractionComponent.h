// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TFInteractableInterface.h"
#include "TFInteractionComponent.generated.h"

class ATFPlayerCharacter;

/** Delegate for interaction events */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionChanged, AActor*, InteractableActor, FInteractionData, InteractionData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionCompleted, AActor*, InteractedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionLost);

/**
 * Interaction Component
 * Handles detection and interaction with interactable objects in the world
 * Attach to player character to enable interaction system
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TFCHARACTERS_API UTFInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

private:

#pragma region Detection Settings

	/** Maximum distance for interaction raycast */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Detection", meta = (AllowPrivateAccess = "true", ClampMin = "50.0", ClampMax = "1000.0"))
	float InteractionDistance = 300.0f;

	/** Radius for sphere trace (0 = line trace from head socket) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Detection", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "50.0"))
	float InteractionRadius = 0.0f;

	/** How often to check for interactables (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Detection", meta = (AllowPrivateAccess = "true", ClampMin = "0.01", ClampMax = "0.5"))
	float DetectionTickRate = 0.1f;

	/** Trace channel for interaction detection */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Detection", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ECollisionChannel> InteractionTraceChannel = ECC_Visibility;

	/** Should trace complex collision */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Detection", meta = (AllowPrivateAccess = "true"))
	bool bTraceComplex = false;

	/** Debug draw interaction traces */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Debug", meta = (AllowPrivateAccess = "true"))
	bool bDebugDraw = true;

#pragma endregion Detection Settings

#pragma region Hold Interaction

	/** Enable hold-to-interact for objects with duration > 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Hold", meta = (AllowPrivateAccess = "true"))
	bool bEnableHoldInteraction = true;

	/** Current hold progress (0.0 - 1.0) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|Hold", meta = (AllowPrivateAccess = "true"))
	float HoldProgress = 0.0f;

	/** Is currently holding interact button */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|Hold", meta = (AllowPrivateAccess = "true"))
	bool bIsHolding = false;

	/** Timer for hold interaction */
	float HoldTimer = 0.0f;

	/** Required hold duration for current interaction */
	float RequiredHoldDuration = 0.0f;

#pragma endregion Hold Interaction

#pragma region State

	/** Currently focused interactable actor */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|State", meta = (AllowPrivateAccess = "true"))
	AActor* CurrentInteractable = nullptr;

	/** Previously focused actor (for focus change detection) */
	UPROPERTY()
	AActor* PreviousInteractable = nullptr;

	/** Cached owner character reference */
	UPROPERTY()
	ATFPlayerCharacter* OwnerCharacter = nullptr;

	/** Timer handle for detection tick */
	FTimerHandle DetectionTimerHandle;

	/** Current interaction data */
	FInteractionData CurrentInteractionData;

#pragma endregion State

protected:

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Perform interaction trace and update focused actor */
	void PerformInteractionCheck();

	/** Get start and end points for interaction trace */
	bool GetTracePoints(FVector& TraceStart, FVector& TraceEnd) const;

	/** Process hit result and update current interactable */
	void ProcessHitResult(const FHitResult& HitResult);

	/** Update focused actor (handle focus changes) */
	void UpdateFocusedActor(AActor* NewFocus);

	/** Clear current focus */
	void ClearFocus();

	/** Update hold interaction progress */
	void UpdateHoldInteraction(float DeltaTime);

	/** Complete hold interaction */
	void CompleteHoldInteraction();

	/** Cancel hold interaction */
	void CancelHoldInteraction();

public:

	UTFInteractionComponent();

#pragma region Delegates

	/** Called when focused interactable changes */
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractionChanged OnInteractionChanged;

	/** Called when interaction is successfully completed */
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractionCompleted OnInteractionCompleted;

	/** Called when interaction focus is lost */
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractionLost OnInteractionLost;

#pragma endregion Delegates

#pragma region Interaction API

	/**
	 * Attempt to interact with currently focused object
	 * Call this from input binding
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact();

	/**
	 * Start hold interaction (call on button press)
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void StartHoldInteraction();

	/**
	 * Stop hold interaction (call on button release)
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void StopHoldInteraction();

	/**
	 * Force interaction with specific actor (bypass trace)
	 * @param Actor - Actor to interact with
	 * @return True if interaction succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool InteractWithActor(AActor* Actor);

	/**
	 * Enable/disable interaction detection
	 * @param bEnabled - Enable or disable
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractionEnabled(bool bEnabled);

#pragma endregion Interaction API

#pragma region Queries

	/**
	 * Get currently focused interactable actor
	 * @return Current interactable or nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	FORCEINLINE AActor* GetCurrentInteractable() const { return CurrentInteractable; }

	/**
	 * Check if currently focusing an interactable
	 * @return True if focusing something
	 */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	FORCEINLINE bool HasInteractable() const { return CurrentInteractable != nullptr; }

	/**
	 * Get current interaction data
	 * @return Interaction data for current focus
	 */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	FORCEINLINE FInteractionData GetCurrentInteractionData() const { return CurrentInteractionData; }

	/**
	 * Check if currently holding interact button
	 * @return True if holding
	 */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	FORCEINLINE bool IsHolding() const { return bIsHolding; }

	/**
	 * Get hold interaction progress (0.0 - 1.0)
	 * @return Progress percentage
	 */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	FORCEINLINE float GetHoldProgress() const { return HoldProgress; }

#pragma endregion Queries

#pragma region Configuration

	/**
	 * Set interaction distance
	 * @param NewDistance - New maximum interaction distance
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractionDistance(float NewDistance);

	/**
	 * Set detection tick rate
	 * @param NewRate - How often to check for interactables (seconds)
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetDetectionTickRate(float NewRate);

#pragma endregion Configuration
};
