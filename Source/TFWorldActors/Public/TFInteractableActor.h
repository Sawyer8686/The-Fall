// TFInteractableActor.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TFInteractableInterface.h"
#include "TFInteractableActor.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UTexture2D;

UCLASS()
class TFWORLDACTORS_API ATFInteractableActor : public AActor, public ITFInteractableInterface
{
	GENERATED_BODY()

protected:

#pragma region Components

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComponent;

#pragma endregion Components

#pragma region Interaction Settings

	UPROPERTY(EditAnywhere, Category = "Interaction")
	UTexture2D* InteractionIcon = nullptr;

	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float InteractionDuration = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (ClampMin = "50.0", ClampMax = "1000.0"))
	float MaxInteractionDistance = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	bool bCanInteract = true;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	bool bIsReusable = true;

	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	bool bHasBeenUsed = false;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	int32 MaxUses = -1;

	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	int32 CurrentUses = 0;

#pragma endregion Interaction Settings

	bool CanBeUsedAgain() const;

public:

	ATFInteractableActor();

#pragma region Interface Implementation

	virtual bool Interact(APawn* InstigatorPawn) override;
	virtual FInteractionData GetInteractionData(APawn* InstigatorPawn) const override;
	virtual bool CanInteract(APawn* InstigatorPawn) const override;
	virtual float GetInteractionDistance() const override;

#pragma endregion Interface Implementation

#pragma region Events

	virtual void OnInteracted(APawn* InstigatorPawn) {}

#pragma endregion Events

#pragma region Accessors

	UStaticMeshComponent* GetMeshComponent() const { return MeshComponent; }
	void SetCanInteract(bool bNewCanInteract);
	void ResetUses();

#pragma endregion Accessors
};
