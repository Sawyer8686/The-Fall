
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TFInteractableInterface.generated.h"



USTRUCT(BlueprintType)
struct FInteractionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText InteractionText = FText::FromString("Interact");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText SecondaryText = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bCanInteract = true;

	FInteractionData()
		: InteractionText(FText::FromString("Interact"))
		, SecondaryText(FText::GetEmpty())
		, InteractionDuration(0.0f)
		, bCanInteract(true)
	{
	}
};

UINTERFACE(MinimalAPI, Blueprintable)
class UTFInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class INTERFACES_API ITFInteractableInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool Interact(APawn* InstigatorPawn);
	virtual bool Interact_Implementation(APawn* InstigatorPawn) { return false; }

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FInteractionData GetInteractionData(APawn* InstigatorPawn) const;
	virtual FInteractionData GetInteractionData_Implementation(APawn* InstigatorPawn) const
	{
		return FInteractionData();
	}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(APawn* InstigatorPawn) const;
	virtual bool CanInteract_Implementation(APawn* InstigatorPawn) const { return true; }

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	float GetInteractionDistance() const;
	virtual float GetInteractionDistance_Implementation() const { return 200.0f; }
};
