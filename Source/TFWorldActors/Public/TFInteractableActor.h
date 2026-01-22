// TFInteractableActor.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TFInteractableInterface.h"
#include "TFInteractableActor.generated.h"

class UStaticMeshComponent;
class USceneComponent;


UCLASS()
class TFWORLDACTORS_API ATFInteractableActor : public AActor, public ITFInteractableInterface
{
	GENERATED_BODY()

protected:

#pragma region Data-Driven Config

	UPROPERTY(EditAnywhere, Category = "Interactable|Config")
	FName InteractableID = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Interactable|Config")
	bool bUseDataDrivenConfig = true;

#pragma endregion Data-Driven Config

#pragma region Components

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComponent;

#pragma endregion Components

#pragma region Interaction Settings

	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (EditCondition = "!bUseDataDrivenConfig", EditConditionHides, ClampMin = "50.0", ClampMax = "1000.0"))
	float MaxInteractionDistance = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (EditCondition = "!bUseDataDrivenConfig", EditConditionHides))
	bool bCanInteract = true;

#pragma endregion Interaction Settings

	virtual void BeginPlay() override;
	virtual void LoadConfigFromINI();

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
	FORCEINLINE FName GetInteractableID() const { return InteractableID; }

#pragma endregion Accessors
};
