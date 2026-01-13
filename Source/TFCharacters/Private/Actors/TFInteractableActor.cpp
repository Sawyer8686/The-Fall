// Fill out your copyright notice in the Description page of Project Settings.

#include "TFInteractableActor.h"
#include "TFPlayerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

ATFInteractableActor::ATFInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root component
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Create mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(Root);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// Create interaction sphere (optional, for proximity detection)
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(Root);
	InteractionSphere->SetSphereRadius(MaxInteractionDistance);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ATFInteractableActor::BeginPlay()
{
	Super::BeginPlay();

	// Update interaction sphere radius
	if (InteractionSphere)
	{
		InteractionSphere->SetSphereRadius(MaxInteractionDistance);
	}
}

void ATFInteractableActor::ApplyHighlight()
{
	if (!bEnableHighlight || !MeshComponent)
	{
		return;
	}

	// Enable custom depth for outline effect
	MeshComponent->SetRenderCustomDepth(true);
	MeshComponent->SetCustomDepthStencilValue(1);

	bIsHighlighted = true;
}

void ATFInteractableActor::RemoveHighlight()
{
	if (!MeshComponent)
	{
		return;
	}

	// Disable custom depth
	MeshComponent->SetRenderCustomDepth(false);
	MeshComponent->SetCustomDepthStencilValue(0);

	bIsHighlighted = false;
}

bool ATFInteractableActor::CanBeUsedAgain() const
{
	// Infinite uses
	if (MaxUses < 0)
	{
		return true;
	}

	// Check use count
	return CurrentUses < MaxUses;
}

bool ATFInteractableActor::Interact_Implementation(ATFPlayerCharacter* InstigatorCharacter)
{
	if (!CanInteract_Implementation(InstigatorCharacter))
	{
		return false;
	}

	// Increment use count
	CurrentUses++;
	bHasBeenUsed = true;

	// Check if can still be used
	if (!CanBeUsedAgain())
	{
		bCanInteract = false;
	}

	// Call blueprint event
	OnInteracted(InstigatorCharacter);

	return true;
}

FInteractionData ATFInteractableActor::GetInteractionData_Implementation(ATFPlayerCharacter* InstigatorCharacter) const
{
	FInteractionData Data;
	Data.InteractionText = InteractionText;
	Data.SecondaryText = SecondaryText;
	Data.InteractionIcon = InteractionIcon;
	Data.InteractionDuration = InteractionDuration;
	Data.bCanInteract = bCanInteract;

	return Data;
}

bool ATFInteractableActor::CanInteract_Implementation(ATFPlayerCharacter* InstigatorCharacter) const
{
	if (!bCanInteract)
	{
		return false;
	}

	// Check if reusable
	if (!bIsReusable && bHasBeenUsed)
	{
		return false;
	}

	// Check use count
	if (!CanBeUsedAgain())
	{
		return false;
	}

	return true;
}

void ATFInteractableActor::OnBeginFocus_Implementation(ATFPlayerCharacter* InstigatorCharacter)
{
	// Apply highlight
	ApplyHighlight();

	// Call blueprint event
	OnFocusBegin(InstigatorCharacter);
}

void ATFInteractableActor::OnEndFocus_Implementation(ATFPlayerCharacter* InstigatorCharacter)
{
	// Remove highlight
	RemoveHighlight();

	// Call blueprint event
	OnFocusEnd(InstigatorCharacter);
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