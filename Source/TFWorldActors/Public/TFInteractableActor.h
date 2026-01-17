// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TFInteractableInterface.h"
#include "TFInteractableActor.generated.h"

class UStaticMeshComponent;
class ATFPlayerCharacter;

UCLASS(Blueprintable)
class TFWORLDACTORS_API ATFInteractableActor : public AActor, public ITFInteractableInterface
{
	GENERATED_BODY()

protected:

#pragma region Components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;


#pragma endregion Components

#pragma region Interaction Settings

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText InteractionText = FText::FromString("Interact");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText SecondaryText = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	UTexture2D* InteractionIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float InteractionDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (ClampMin = "50.0", ClampMax = "1000.0"))
	float MaxInteractionDistance = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bCanInteract = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bIsReusable = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bHasBeenUsed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	int32 MaxUses = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	int32 CurrentUses = 0;

#pragma endregion Interaction Settings

	virtual void BeginPlay() override;
	bool CanBeUsedAgain() const;

public:

	ATFInteractableActor();

#pragma region Interface Implementation

	virtual bool Interact_Implementation(APawn* InstigatorPawn) override;
	virtual FInteractionData GetInteractionData_Implementation(APawn* InstigatorPawn) const override;
	virtual bool CanInteract_Implementation(APawn* InstigatorPawn) const override;
	virtual void OnBeginFocus_Implementation(APawn* InstigatorPawn) override;
	virtual void OnEndFocus_Implementation(APawn* InstigatorPawn) override;
	virtual float GetInteractionDistance_Implementation() const override;

#pragma endregion Interface Implementation

#pragma region Blueprint Events

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnInteracted(ATFPlayerCharacter* InstigatorCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnFocusBegin(ATFPlayerCharacter* InstigatorCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnFocusEnd(ATFPlayerCharacter* InstigatorCharacter);

#pragma endregion Blueprint Events

#pragma region Accessors

	UFUNCTION(BlueprintPure, Category = "Components")
	FORCEINLINE UStaticMeshComponent* GetMeshComponent() const { return MeshComponent; }

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetCanInteract(bool bNewCanInteract);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractionText(FText NewText);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ResetUses();

#pragma endregion Accessors
};
