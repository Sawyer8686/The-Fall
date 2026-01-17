// Fill out your copyright notice in the Description page of Project Settings.

#include "TFInteractableActor.h"
#include "TFPlayerCharacter.h"
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

void ATFInteractableActor::BeginPlay()
{
	Super::BeginPlay();

}


bool ATFInteractableActor::CanBeUsedAgain() const
{
	if (MaxUses < 0)
	{
		return true;
	}
	return CurrentUses < MaxUses;
}

bool ATFInteractableActor::Interact_Implementation(APawn* InstigatorPawn)
{
	if (!CanInteract_Implementation(InstigatorPawn))
	{
		return false;
	}

	CurrentUses++;
	bHasBeenUsed = true;

	if (!CanBeUsedAgain())
	{
		bCanInteract = false;
	}

	ATFPlayerCharacter* PlayerCharacter = Cast<ATFPlayerCharacter>(InstigatorPawn);
	OnInteracted(PlayerCharacter);

	return true;
}

FInteractionData ATFInteractableActor::GetInteractionData_Implementation(APawn* InstigatorPawn) const
{
	FInteractionData Data;
	Data.InteractionText = InteractionText;
	Data.SecondaryText = SecondaryText;
	Data.InteractionDuration = InteractionDuration;
	Data.bCanInteract = bCanInteract;

	return Data;
}

bool ATFInteractableActor::CanInteract_Implementation(APawn* InstigatorPawn) const
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

float ATFInteractableActor::GetInteractionDistance_Implementation() const
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
