// Fill out your copyright notice in the Description page of Project Settings.

#include "TFInteractableActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

ATFInteractableActor::ATFInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(Root);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

}

bool ATFInteractableActor::CanBeUsedAgain() const
{
	if (MaxUses < 0)
	{
		return true;
	}
	return CurrentUses < MaxUses;
}

bool ATFInteractableActor::Interact(APawn* InstigatorPawn)
{
	if (!CanInteract(InstigatorPawn))
	{
		return false;
	}

	CurrentUses++;
	bHasBeenUsed = true;

	if (!CanBeUsedAgain())
	{
		bCanInteract = false;
	}

	OnInteracted(InstigatorPawn);

	return true;
}

FInteractionData ATFInteractableActor::GetInteractionData(APawn* InstigatorPawn) const
{
	FInteractionData Data;
	Data.InteractionText = InteractionText;
	Data.SecondaryText = SecondaryText;
	Data.InteractionDuration = InteractionDuration;
	Data.bCanInteract = bCanInteract;

	return Data;
}

bool ATFInteractableActor::CanInteract(APawn* InstigatorPawn) const
{
	if (!bCanInteract)
	{
		return false;
	}

	if (!bIsReusable && bHasBeenUsed)
	{
		return false;
	}

	if (!CanBeUsedAgain())
	{
		return false;
	}

	return true;
}

float ATFInteractableActor::GetInteractionDistance() const
{
	return MaxInteractionDistance;
}

void ATFInteractableActor::SetCanInteract(bool bNewCanInteract)
{
	bCanInteract = bNewCanInteract;
}

void ATFInteractableActor::SetInteractionText(FText NewText)
{
	InteractionText = NewText;
}

void ATFInteractableActor::ResetUses()
{
	CurrentUses = 0;
	bHasBeenUsed = false;
	bCanInteract = true;
}
