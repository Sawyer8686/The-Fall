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

	OwnerCharacter = Cast<ATFPlayerCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("TFInteractionComponent: Owner is not a TFPlayerCharacter!"));
		return;
	}

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
}

void UTFInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DetectionTimerHandle);
	}

	ClearFocus();

	Super::EndPlay(EndPlayReason);
}

void UTFInteractionComponent::PerformInteractionCheck()
{
	if (!OwnerCharacter)
	{
		return;
	}

	FVector TraceStart, TraceEnd;
	if (!GetTracePoints(TraceStart, TraceEnd))
	{
		ClearFocus();
		return;
	}

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	QueryParams.bTraceComplex = bTraceComplex;

	bool bHit = false;

	if (InteractionRadius > 0.0f)
	{
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
		bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			TraceStart,
			TraceEnd,
			InteractionTraceChannel,
			QueryParams
		);
	}

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
	if (CurrentInteractable == NewFocus)
	{
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

	if (CurrentInteractable)
	{
		ITFInteractableInterface* OldInteractable = Cast<ITFInteractableInterface>(CurrentInteractable);
		if (OldInteractable)
		{
			OldInteractable->Execute_OnEndFocus(CurrentInteractable, OwnerCharacter);
		}
	}

	PreviousInteractable = CurrentInteractable;
	CurrentInteractable = NewFocus;

	if (CurrentInteractable)
	{
		ITFInteractableInterface* NewInteractable = Cast<ITFInteractableInterface>(CurrentInteractable);
		if (NewInteractable)
		{
			CurrentInteractionData = NewInteractable->Execute_GetInteractionData(CurrentInteractable, OwnerCharacter);

			NewInteractable->Execute_OnBeginFocus(CurrentInteractable, OwnerCharacter);

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

	if (!CurrentInteractable->Implements<UTFInteractableInterface>())
	{
		ClearFocus();
		return;
	}

	const bool bSuccess = ITFInteractableInterface::Execute_Interact(CurrentInteractable, OwnerCharacter);

	if (bSuccess)
	{
		OnInteractionCompleted.Broadcast(CurrentInteractable);

		if (CurrentInteractable->Implements<UTFPickupableInterface>())
		{
			if (ITFPickupableInterface::Execute_ShouldDestroyOnPickup(CurrentInteractable))
			{
				ClearFocus();
			}
		}
	}
}

bool UTFInteractionComponent::InteractWithActor(AActor* Actor)
{
	if (!Actor || !OwnerCharacter)
	{
		return false;
	}

	if (!Actor->Implements<UTFInteractableInterface>())
	{
		return false;
	}

	ITFInteractableInterface* Interactable = Cast<ITFInteractableInterface>(Actor);
	if (!Interactable)
	{
		return false;
	}

	if (!Interactable->Execute_CanInteract(Actor, OwnerCharacter))
	{
		return false;
	}

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
