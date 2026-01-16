// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/TFInteractableInterface.h"
#include "TFInteractableActor.generated.h"

class UStaticMeshComponent;
class USphereComponent;

UCLASS(Blueprintable)
class TFCHARACTERS_API ATFInteractableActor : public AActor, public ITFInteractableInterface
{
	GENERATED_BODY()

protected:

#pragma region Components

	/** Root scene component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Root;

	/** Visual mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	/** Optional collision sphere for detection */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* InteractionSphere;

#pragma endregion Components

#pragma region Interaction Settings

	/** Display text for interaction prompt */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText InteractionText = FText::FromString("Interact");

	/** Secondary text (e.g., status or requirements) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText SecondaryText = FText::GetEmpty();

	/** Icon for interaction UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	UTexture2D* InteractionIcon = nullptr;

	/** Time required to hold interact button (0 = instant) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float InteractionDuration = 0.0f;

	/** Maximum distance player can be to interact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (ClampMin = "50.0", ClampMax = "1000.0"))
	float MaxInteractionDistance = 200.0f;

	/** Can currently be interacted with */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bCanInteract = true;

	/** Can be used multiple times */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bIsReusable = true;

	/** Has been used at least once */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bHasBeenUsed = false;

	/** Number of times this can be used (-1 = infinite) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	int32 MaxUses = -1;

	/** Current use count */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	int32 CurrentUses = 0;

#pragma endregion Interaction Settings

#pragma region Highlight Settings

	/** Enable highlight when player looks at object */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Highlight")
	bool bEnableHighlight = true;

	/** Highlight color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Highlight")
	FLinearColor HighlightColor = FLinearColor(0.0f, 1.0f, 0.5f, 1.0f);

	/** Highlight intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Highlight", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float HighlightIntensity = 3.0f;

	/** Is currently highlighted */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|Highlight")
	bool bIsHighlighted = false;

#pragma endregion Highlight Settings

	virtual void BeginPlay() override;

	/** Apply highlight effect to mesh */
	virtual void ApplyHighlight();

	/** Remove highlight effect from mesh */
	virtual void RemoveHighlight();

	/** Check if can be used again */
	bool CanBeUsedAgain() const;

public:

	ATFInteractableActor();

#pragma region Interface Implementation

	/** Execute interaction */
	virtual bool Interact_Implementation(ATFPlayerCharacter* InstigatorCharacter) override;

	/** Get interaction data */
	virtual FInteractionData GetInteractionData_Implementation(ATFPlayerCharacter* InstigatorCharacter) const override;

	/** Check if can interact */
	virtual bool CanInteract_Implementation(ATFPlayerCharacter* InstigatorCharacter) const override;

	/** Begin focus */
	virtual void OnBeginFocus_Implementation(ATFPlayerCharacter* InstigatorCharacter) override;

	/** End focus */
	virtual void OnEndFocus_Implementation(ATFPlayerCharacter* InstigatorCharacter) override;

	/** Get interaction distance */
	virtual float GetInteractionDistance_Implementation() const override;

#pragma endregion Interface Implementation

#pragma region Blueprint Events

	/** Called when interaction occurs (implement in BP) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnInteracted(ATFPlayerCharacter* InstigatorCharacter);

	/** Called when player starts looking at this object (implement in BP) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnFocusBegin(ATFPlayerCharacter* InstigatorCharacter);

	/** Called when player stops looking at this object (implement in BP) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnFocusEnd(ATFPlayerCharacter* InstigatorCharacter);

#pragma endregion Blueprint Events

#pragma region Accessors

	/** Get mesh component */
	UFUNCTION(BlueprintPure, Category = "Components")
	FORCEINLINE UStaticMeshComponent* GetMeshComponent() const { return MeshComponent; }

	/** Set if can interact */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetCanInteract(bool bNewCanInteract);

	/** Set interaction text */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractionText(FText NewText);

	/** Reset use count */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ResetUses();

#pragma endregion Accessors
};