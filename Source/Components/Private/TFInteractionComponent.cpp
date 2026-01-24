// Copyright TF Project. All Rights Reserved.

#include "TFInteractionComponent.h"
#include "TFTypes.h"

#include "GameFramework/Character.h"
#include "TFInteractableInterface.h"
#include "TFPickupableInterface.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UTFInteractionComponent::UTFInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UTFInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTFInteraction, Error, TEXT("TFInteractionComponent: Owner is not a Character!"));
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DetectionTimerHandle,
			this,
			&UTFInteractionComponent::PerformInteractionCheck,
			DetectionTickRate,
			true
		);
	}
}

void UTFInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DetectionTimerHandle);
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

	UWorld* World = GetWorld();
	if (!World)
	{
		ClearFocus();
		return;
	}

	FVector TraceStart, TraceEnd;
	if (!GetTracePoints(TraceStart, TraceEnd))
	{
		ClearFocus();
		return;
	}

	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(TFInteractionTrace), bTraceComplex);
	QueryParams.AddIgnoredActor(OwnerCharacter);

	bool bHit = false;

	if (InteractionRadius > 0.0f)
	{
		bHit = World->SweepSingleByChannel(
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
		bHit = World->LineTraceSingleByChannel(
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

	USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
	if (!MeshComp)
	{
		return false;
	}

	AController* Controller = OwnerCharacter->GetController();
	if (!Controller)
	{
		return false;
	}

	TraceStart = MeshComp->GetSocketLocation(TEXT("head"));

	const FVector ForwardVector = Controller->GetControlRotation().Vector();
	TraceEnd = TraceStart + (ForwardVector * InteractionDistance);

	return true;
}

void UTFInteractionComponent::ProcessHitResult(const FHitResult& HitResult)
{
	AActor* HitActor = HitResult.GetActor();
	if (!HitActor || !OwnerCharacter)
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

	float ActorMaxDistance = Interactable->GetInteractionDistance();
	if (HitResult.Distance > ActorMaxDistance)
	{
		ClearFocus();
		return;
	}

	if (!Interactable->CanInteract(OwnerCharacter))
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
		if (CurrentInteractable)
		{
			if (ITFInteractableInterface* Interactable = Cast<ITFInteractableInterface>(CurrentInteractable))
			{
				FInteractionData NewData = Interactable->GetInteractionData(OwnerCharacter);

				if (
					CurrentInteractionData.bCanInteract != NewData.bCanInteract)
				{
					CurrentInteractionData = NewData;
					OnInteractionChanged.Broadcast(CurrentInteractable, CurrentInteractionData);
				}
			}
		}
		return;
	}

	PreviousInteractable = CurrentInteractable;
	CurrentInteractable = NewFocus;

	if (CurrentInteractable)
	{
		if (ITFInteractableInterface* Interactable = Cast<ITFInteractableInterface>(CurrentInteractable))
		{
			CurrentInteractionData = Interactable->GetInteractionData(OwnerCharacter);
			OnInteractionChanged.Broadcast(CurrentInteractable, CurrentInteractionData);
		}
	}
	else
	{
		CurrentInteractionData = FInteractionData();
		OnInteractionLost.Broadcast();
	}
}

void UTFInteractionComponent::ClearFocus()
{
	if (!CurrentInteractable)
	{
		return;
	}

	PreviousInteractable = CurrentInteractable;
	CurrentInteractable = nullptr;
	CurrentInteractionData = FInteractionData();

	OnInteractionLost.Broadcast();
}

void UTFInteractionComponent::Interact()
{
	if (!OwnerCharacter || !CurrentInteractable)
	{
		return;
	}

	ITFInteractableInterface* Interactable = Cast<ITFInteractableInterface>(CurrentInteractable);
	if (!Interactable)
	{
		ClearFocus();
		return;
	}

	const bool bSuccess = Interactable->Interact(OwnerCharacter);

	if (bSuccess)
	{
		OnInteractionCompleted.Broadcast(CurrentInteractable);

		if (ITFPickupableInterface* Pickupable = Cast<ITFPickupableInterface>(CurrentInteractable))
		{
			if (Pickupable->ShouldDestroyOnPickup())
			{
				ClearFocus();
			}
		}
	}
}

bool UTFInteractionComponent::InteractWithActor(AActor* Actor)
{
	if (!OwnerCharacter || !Actor)
	{
		return false;
	}

	ITFInteractableInterface* Interactable = Cast<ITFInteractableInterface>(Actor);
	if (!Interactable)
	{
		return false;
	}

	if (!Interactable->CanInteract(OwnerCharacter))
	{
		return false;
	}

	const bool bSuccess = Interactable->Interact(OwnerCharacter);

	if (bSuccess)
	{
		OnInteractionCompleted.Broadcast(Actor);
	}

	return bSuccess;
}

void UTFInteractionComponent::SetInteractionEnabled(bool bEnabled)
{
	if (UWorld* World = GetWorld())
	{
		if (bEnabled)
		{
			World->GetTimerManager().SetTimer(
				DetectionTimerHandle,
				this,
				&UTFInteractionComponent::PerformInteractionCheck,
				DetectionTickRate,
				true
			);
		}
		else
		{
			World->GetTimerManager().ClearTimer(DetectionTimerHandle);
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

	if (UWorld* World = GetWorld())
	{
		if (DetectionTimerHandle.IsValid())
		{
			World->GetTimerManager().ClearTimer(DetectionTimerHandle);
		}

		World->GetTimerManager().SetTimer(
			DetectionTimerHandle,
			this,
			&UTFInteractionComponent::PerformInteractionCheck,
			DetectionTickRate,
			true
		);
	}
}
