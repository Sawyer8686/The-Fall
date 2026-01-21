// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TFInteractableInterface.h"
#include "TFInteractionComponent.generated.h"

class ACharacter;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInteractionChanged, AActor*, FInteractionData);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInteractionCompleted, AActor*);
DECLARE_MULTICAST_DELEGATE(FOnInteractionLost);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COMPONENTS_API UTFInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

private:

#pragma region Detection Settings

	UPROPERTY(EditAnywhere, Category = "Interaction|Detection", meta = (ClampMin = "50.0", ClampMax = "1000.0"))
	float InteractionDistance = 300.0f;

	//UPROPERTY(EditAnywhere, Category = "Interaction|Detection", meta = (ClampMin = "0.0", ClampMax = "50.0"))
	//float InteractionRadius = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Interaction|Detection", meta = (ClampMin = "0.01", ClampMax = "0.5"))
	float DetectionTickRate = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Interaction|Detection")
	TEnumAsByte<ECollisionChannel> InteractionTraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, Category = "Interaction|Detection")
	bool bTraceComplex = false;

#pragma endregion Detection Settings

#pragma region State

	UPROPERTY(VisibleAnywhere, Category = "Interaction|State")
	AActor* CurrentInteractable = nullptr;

	UPROPERTY()
	AActor* PreviousInteractable = nullptr;

	UPROPERTY()
	ACharacter* OwnerCharacter = nullptr;

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

	FOnInteractionChanged OnInteractionChanged;
	FOnInteractionCompleted OnInteractionCompleted;
	FOnInteractionLost OnInteractionLost;

#pragma endregion Delegates

#pragma region Interaction API

	void Interact();
	bool InteractWithActor(AActor* Actor);
	void SetInteractionEnabled(bool bEnabled);

#pragma endregion Interaction API

#pragma region Queries

	AActor* GetCurrentInteractable() const { return CurrentInteractable; }
	bool HasInteractable() const { return CurrentInteractable != nullptr; }
	FInteractionData GetCurrentInteractionData() const { return CurrentInteractionData; }

#pragma endregion Queries

#pragma region Configuration

	void SetInteractionDistance(float NewDistance);
	void SetDetectionTickRate(float NewRate);

#pragma endregion Configuration
};
