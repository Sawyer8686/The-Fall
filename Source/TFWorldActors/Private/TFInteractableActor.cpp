// Copyright TF Project. All Rights Reserved.

#include "TFInteractableActor.h"
#include "TFTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Misc/ConfigCacheIni.h"

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

	if (bUseDataDrivenConfig && !InteractableID.IsNone())
	{
		LoadConfigFromINI();
	}
}

void ATFInteractableActor::LoadConfigFromINI()
{
	const FString SectionName = InteractableID.ToString();
	FString ConfigFilePath;

	if (!TFConfigUtils::LoadINISection(TEXT("InteractableConfig.ini"), SectionName, ConfigFilePath, LogTFInteraction, true))
	{
		return;
	}

	UE_LOG(LogTFInteraction, Log, TEXT("ATFInteractableActor: Loading config for InteractableID '%s'"), *SectionName);

	GConfig->GetFloat(*SectionName, TEXT("MaxInteractionDistance"), MaxInteractionDistance, ConfigFilePath);
	GConfig->GetBool(*SectionName, TEXT("bCanInteract"), bCanInteract, ConfigFilePath);

	MaxInteractionDistance = FMath::Clamp(MaxInteractionDistance, 50.0f, 1000.0f);

	UE_LOG(LogTFInteraction, Log, TEXT("ATFInteractableActor: Config loaded successfully for InteractableID '%s'"), *SectionName);
}



bool ATFInteractableActor::Interact(APawn* InstigatorPawn)
{
	if (!CanInteract(InstigatorPawn))
	{
		return false;
	}

	OnInteracted(InstigatorPawn);

	return true;
}

FInteractionData ATFInteractableActor::GetInteractionData(APawn* InstigatorPawn) const
{
	FInteractionData Data;
	Data.bCanInteract = bCanInteract;

	return Data;
}

bool ATFInteractableActor::CanInteract(APawn* InstigatorPawn) const
{
	if (!bCanInteract)
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

