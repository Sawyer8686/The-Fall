
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TFInteractableInterface.generated.h"



USTRUCT()
struct FInteractionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Interaction")
	float InteractionDuration = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	bool bCanInteract = true;

	FInteractionData()
		: InteractionDuration(0.0f)
		, bCanInteract(true)
	{
	}
};

UINTERFACE(MinimalAPI)
class UTFInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class INTERFACES_API ITFInteractableInterface
{
	GENERATED_BODY()

public:

	virtual bool Interact(APawn* InstigatorPawn) { return false; }

	virtual FInteractionData GetInteractionData(APawn* InstigatorPawn) const
	{
		return FInteractionData();
	}

	virtual bool CanInteract(APawn* InstigatorPawn) const { return true; }

	virtual float GetInteractionDistance() const { return 200.0f; }
};
