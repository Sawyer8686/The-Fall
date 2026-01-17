// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TFInteractableInterface.h"
#include "TFInteractionComponent.generated.h"

class ATFPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionChanged, AActor*, InteractableActor, FInteractionData, InteractionData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionCompleted, AActor*, InteractedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionLost);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TFCHARACTERS_API UTFInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

private:

#pragma region Detection Settings

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Detection", meta = (AllowPrivateAccess = "true", ClampMin = "50.0", ClampMax = "1000.0"))
	float InteractionDistance = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Detection", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "50.0"))
	float InteractionRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Detection", meta = (AllowPrivateAccess = "true", ClampMin = "0.01", ClampMax = "0.5"))
	float DetectionTickRate = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Detection", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ECollisionChannel> InteractionTraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Detection", meta = (AllowPrivateAccess = "true"))
	bool bTraceComplex = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Debug", meta = (AllowPrivateAccess = "true"))
	bool bDebugDraw = true;

#pragma endregion Detection Settings

#pragma region State

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|State", meta = (AllowPrivateAccess = "true"))
	AActor* CurrentInteractable = nullptr;

	UPROPERTY()
	AActor* PreviousInteractable = nullptr;

	UPROPERTY()
	ATFPlayerCharacter* OwnerCharacter = nullptr;

	FTimerHandle DetectionTimerHandle;

	FInteractionData CurrentInteractionData;

#pragma endregion State

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void PerformInteractionCheck();
	bool GetTracePoints(FVector& TraceStart, FVector& TraceEnd) const;
	void ProcessHitResult(const FHitResult& HitResult);
	void UpdateFocusedActor(AActor* NewFocus);
	void ClearFocus();
	

public:

	UTFInteractionComponent();

#pragma region Delegates

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractionChanged OnInteractionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractionCompleted OnInteractionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractionLost OnInteractionLost;

#pragma endregion Delegates

#pragma region Interaction API

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool InteractWithActor(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractionEnabled(bool bEnabled);

#pragma endregion Interaction API

#pragma region Queries

	UFUNCTION(BlueprintPure, Category = "Interaction")
	FORCEINLINE AActor* GetCurrentInteractable() const { return CurrentInteractable; }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	FORCEINLINE bool HasInteractable() const { return CurrentInteractable != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	FORCEINLINE FInteractionData GetCurrentInteractionData() const { return CurrentInteractionData; }

#pragma endregion Queries

#pragma region Configuration

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractionDistance(float NewDistance);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetDetectionTickRate(float NewRate);

#pragma endregion Configuration
};
