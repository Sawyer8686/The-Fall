// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TFInteractableInterface.generated.h"

class ATFPlayerCharacter;

/**
 * Interaction data returned by interactable objects
 */
USTRUCT(BlueprintType)
struct FInteractionData
{
	GENERATED_BODY()

	/** Display text shown to player (e.g., "Press E to Open Door") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText InteractionText = FText::FromString("Interact");

	/** Optional secondary text (e.g., "Locked" or "Requires Key") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText SecondaryText = FText::GetEmpty();

	/** Icon to display in UI (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	UTexture2D* InteractionIcon = nullptr;

	/** Duration of interaction (0 = instant, >0 = hold to interact) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionDuration = 0.0f;

	/** Can this object be interacted with right now */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bCanInteract = true;

	FInteractionData()
		: InteractionText(FText::FromString("Interact"))
		, SecondaryText(FText::GetEmpty())
		, InteractionIcon(nullptr)
		, InteractionDuration(0.0f)
		, bCanInteract(true)
	{
	}
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UTFInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for objects that can be interacted with by the player
 * Implement this interface on any actor that should respond to player interaction
 */
class TFCHARACTERS_API ITFInteractableInterface
{
	GENERATED_BODY()

public:

	/**
	 * Called when player interacts with this object
	 * @param InstigatorCharacter - The character performing the interaction
	 * @return True if interaction was successful
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool Interact(ATFPlayerCharacter* InstigatorCharacter);
	virtual bool Interact_Implementation(ATFPlayerCharacter* InstigatorCharacter) { return false; }

	/**
	 * Get interaction data to display to player
	 * @param InstigatorCharacter - The character looking at this object
	 * @return Interaction data with display information
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FInteractionData GetInteractionData(ATFPlayerCharacter* InstigatorCharacter) const;
	virtual FInteractionData GetInteractionData_Implementation(ATFPlayerCharacter* InstigatorCharacter) const
	{
		return FInteractionData();
	}

	/**
	 * Check if this object can currently be interacted with
	 * @param InstigatorCharacter - The character attempting to interact
	 * @return True if interaction is possible
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(ATFPlayerCharacter* InstigatorCharacter) const;
	virtual bool CanInteract_Implementation(ATFPlayerCharacter* InstigatorCharacter) const { return true; }

	/**
	 * Called when player starts looking at this object
	 * @param InstigatorCharacter - The character now focusing on this object
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnBeginFocus(ATFPlayerCharacter* InstigatorCharacter);
	virtual void OnBeginFocus_Implementation(ATFPlayerCharacter* InstigatorCharacter) {}

	/**
	 * Called when player stops looking at this object
	 * @param InstigatorCharacter - The character no longer focusing on this object
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnEndFocus(ATFPlayerCharacter* InstigatorCharacter);
	virtual void OnEndFocus_Implementation(ATFPlayerCharacter* InstigatorCharacter) {}

	/**
	 * Get the interaction distance for this object
	 * @return Maximum distance player can be to interact
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	float GetInteractionDistance() const;
	virtual float GetInteractionDistance_Implementation() const { return 200.0f; }
};