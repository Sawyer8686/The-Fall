// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/TFInteractionComponent.h"
#include "TFPlayerCharacter.h"
#include "TFPickupableInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"

UTFInteractionComponent::UTFInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UTFInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache owner character
	OwnerCharacter = Cast<ATFPlayerCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("TFInteractionComponent: Owner is not a TFPlayerCharacter!"));
		return;
	}

	// Start detection timer
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			DetectionTimerHandle,
			this,
			&UTFInteractionComponent::PerformInteractionCheck,
			DetectionTickRate,
			true
		);
	}
}

void UTFInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update hold interaction if active
	if (bIsHolding && RequiredHoldDuration > 0.0f)
	{
		UpdateHoldInteraction(DeltaTime);
	}
}

void UTFInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clear timer
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DetectionTimerHandle);
	}

	// Clear focus
	ClearFocus();

	Super::EndPlay(EndPlayReason);
}

void UTFInteractionComponent::PerformInteractionCheck()
{
	if (!OwnerCharacter)
	{
		return;
	}

	// Get trace points
	FVector TraceStart, TraceEnd;
	if (!GetTracePoints(TraceStart, TraceEnd))
	{
		ClearFocus();
		return;
	}

	// Perform trace
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	QueryParams.bTraceComplex = bTraceComplex;

	bool bHit = false;

	if (InteractionRadius > 0.0f)
	{
		// Sphere trace
		bHit = GetWorld()->SweepSingleByChannel(
			HitResult,
			TraceStart,
			TraceEnd,
			FQuat::Identity,
			InteractionTraceChannel,
			FCollisionShape::MakeSphere(InteractionRadius),
			QueryParams
		);
	}
	else
	{
		// Line trace
		bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			TraceStart,
			TraceEnd,
			InteractionTraceChannel,
			QueryParams
		);
	}

	// Process result
	if (bHit)
	{
		ProcessHitResult(HitResult);
	}
	else
	{
		ClearFocus();
	}
}

bool UTFInteractionComponent::GetTracePoints(FVector& TraceStart, FVector& TraceEnd) const
{
	if (!OwnerCharacter)
	{
		return false;
	}

	USkeletalMeshComponent * MeshComp = OwnerCharacter->GetMesh();
	if (!MeshComp)
	{
		return false;
	}
	 
		TraceStart = MeshComp->GetSocketLocation(TEXT("head"));
		 
		AController * Controller = OwnerCharacter->GetController();
		 if (!Controller)
		 {
		 return false;
		 }

	FVector ForwardVector = Controller->GetControlRotation().Vector();
	TraceEnd = TraceStart + (ForwardVector * InteractionDistance);

	return true;
}

void UTFInteractionComponent::ProcessHitResult(const FHitResult& HitResult)
{
	AActor* HitActor = HitResult.GetActor();
	if (!HitActor)
	{
		ClearFocus();
		return;
	}

	if (!HitActor->Implements<UTFInteractableInterface>())
	{
		ClearFocus();
		return;
	}

	ITFInteractableInterface* Interactable = Cast<ITFInteractableInterface>(HitActor);
	if (!Interactable)
	{
		ClearFocus();
		return;
	}

	if (!Interactable->Execute_CanInteract(HitActor, OwnerCharacter))
	{
		ClearFocus();
		return;
	}

	UpdateFocusedActor(HitActor);
}

void UTFInteractionComponent::UpdateFocusedActor(AActor* NewFocus)
{
	// Same actor, just update data
	if (CurrentInteractable == NewFocus)
	{
		// Update interaction data
		if (NewFocus)
		{
			ITFInteractableInterface* Interactable = Cast<ITFInteractableInterface>(NewFocus);
			if (Interactable)
			{
				CurrentInteractionData = Interactable->Execute_GetInteractionData(NewFocus, OwnerCharacter);
			}
		}
		return;
	}

	// Focus changed - clear old focus
	if (CurrentInteractable)
	{
		ITFInteractableInterface* OldInteractable = Cast<ITFInteractableInterface>(CurrentInteractable);
		if (OldInteractable)
		{
			OldInteractable->Execute_OnEndFocus(CurrentInteractable, OwnerCharacter);
		}
	}

	// Set new focus
	PreviousInteractable = CurrentInteractable;
	CurrentInteractable = NewFocus;

	if (CurrentInteractable)
	{
		ITFInteractableInterface* NewInteractable = Cast<ITFInteractableInterface>(CurrentInteractable);
		if (NewInteractable)
		{
			// Get interaction data
			CurrentInteractionData = NewInteractable->Execute_GetInteractionData(CurrentInteractable, OwnerCharacter);

			// Notify begin focus
			NewInteractable->Execute_OnBeginFocus(CurrentInteractable, OwnerCharacter);

			// Broadcast event
			OnInteractionChanged.Broadcast(CurrentInteractable, CurrentInteractionData);
		}
	}
	else
	{
		OnInteractionLost.Broadcast();
	}
}

void UTFInteractionComponent::ClearFocus()
{
	// Cancel any active hold interaction when focus is lost
    if (bIsHolding)
	    {
		 CancelHoldInteraction();
		}
	if (CurrentInteractable)
	{
		ITFInteractableInterface* Interactable = Cast<ITFInteractableInterface>(CurrentInteractable);
		if (Interactable)
		{
			Interactable->Execute_OnEndFocus(CurrentInteractable, OwnerCharacter);
		}

		PreviousInteractable = CurrentInteractable;
		CurrentInteractable = nullptr;
		CurrentInteractionData = FInteractionData();

		OnInteractionLost.Broadcast();
	}
}

void UTFInteractionComponent::Interact()
{
	if (!CurrentInteractable || !OwnerCharacter)
	{
		return;
	}

	// Check if this is a hold interaction
	if (bEnableHoldInteraction && CurrentInteractionData.InteractionDuration > 0.0f)
	{
		// Hold interaction handled by StartHoldInteraction/StopHoldInteraction
		return;
	}

	// Instant interaction
	ITFInteractableInterface* Interactable = Cast<ITFInteractableInterface>(CurrentInteractable);
	if (Interactable)
	{
		bool bSuccess = Interactable->Execute_Interact(CurrentInteractable, OwnerCharacter);

		if (bSuccess)
		{
			OnInteractionCompleted.Broadcast(CurrentInteractable);

			// Check if this was a pickupable that should be destroyed
			ITFPickupableInterface* Pickupable = Cast<ITFPickupableInterface>(CurrentInteractable);
			if (Pickupable && Pickupable->Execute_ShouldDestroyOnPickup(CurrentInteractable))
			{
				ClearFocus();
			}
		}
	}
}

void UTFInteractionComponent::StartHoldInteraction()
{
	if (!CurrentInteractable || !bEnableHoldInteraction)
	{
		return;
	}

	// Check if interaction requires holding
	if (CurrentInteractionData.InteractionDuration <= 0.0f)
	{
		// Instant interaction
		Interact();
		return;
	}

	// Start holding
	bIsHolding = true;
	HoldTimer = 0.0f;
	HoldProgress = 0.0f;
	RequiredHoldDuration = CurrentInteractionData.InteractionDuration;
}

void UTFInteractionComponent::StopHoldInteraction()
{
	if (bIsHolding)
	{
		CancelHoldInteraction();
	}
}

void UTFInteractionComponent::UpdateHoldInteraction(float DeltaTime)
{
	if (!bIsHolding || !CurrentInteractable)
	{
		return;
	}

	// Update timer
	HoldTimer += DeltaTime;
	HoldProgress = FMath::Clamp(HoldTimer / RequiredHoldDuration, 0.0f, 1.0f);

	// Check if completed
	if (HoldProgress >= 1.0f)
	{
		CompleteHoldInteraction();
	}
}

void UTFInteractionComponent::CompleteHoldInteraction()
{
	if (!CurrentInteractable)
	{
		CancelHoldInteraction();
		return;
	}

	// Execute interaction
	ITFInteractableInterface* Interactable = Cast<ITFInteractableInterface>(CurrentInteractable);
	if (Interactable)
	{
		bool bSuccess = Interactable->Execute_Interact(CurrentInteractable, OwnerCharacter);

		if (bSuccess)
		{
			OnInteractionCompleted.Broadcast(CurrentInteractable);
		}
	}

	// Reset hold state
	bIsHolding = false;
	HoldTimer = 0.0f;
	HoldProgress = 0.0f;
	RequiredHoldDuration = 0.0f;
}

void UTFInteractionComponent::CancelHoldInteraction()
{
	bIsHolding = false;
	HoldTimer = 0.0f;
	HoldProgress = 0.0f;
	RequiredHoldDuration = 0.0f;
}

bool UTFInteractionComponent::InteractWithActor(AActor* Actor)
{
	if (!Actor || !OwnerCharacter)
	{
		return false;
	}

	// Check if implements interface
	if (!Actor->Implements<UTFInteractableInterface>())
	{
		return false;
	}

	ITFInteractableInterface* Interactable = Cast<ITFInteractableInterface>(Actor);
	if (!Interactable)
	{
		return false;
	}

	// Check if can interact
	if (!Interactable->Execute_CanInteract(Actor, OwnerCharacter))
	{
		return false;
	}

	// Execute interaction
	bool bSuccess = Interactable->Execute_Interact(Actor, OwnerCharacter);

	if (bSuccess)
	{
		OnInteractionCompleted.Broadcast(Actor);
	}

	return bSuccess;
}

void UTFInteractionComponent::SetInteractionEnabled(bool bEnabled)
{
	SetComponentTickEnabled(bEnabled);

	if (GetWorld())
	{
		if (bEnabled)
		{
			GetWorld()->GetTimerManager().SetTimer(
				DetectionTimerHandle,
				this,
				&UTFInteractionComponent::PerformInteractionCheck,
				DetectionTickRate,
				true
			);
		}
		else
		{
			GetWorld()->GetTimerManager().ClearTimer(DetectionTimerHandle);
			ClearFocus();
		}
	}
}

void UTFInteractionComponent::SetInteractionDistance(float NewDistance)
{
	InteractionDistance = FMath::Clamp(NewDistance, 50.0f, 1000.0f);
}

void UTFInteractionComponent::SetDetectionTickRate(float NewRate)
{
	DetectionTickRate = FMath::Clamp(NewRate, 0.01f, 0.5f);

	// Restart timer with new rate
	if (GetWorld() && DetectionTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(DetectionTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(
			DetectionTimerHandle,
			this,
			&UTFInteractionComponent::PerformInteractionCheck,
			DetectionTickRate,
			true
		);
	}
}
