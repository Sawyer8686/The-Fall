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

	// Use helper function - silent on missing since derived classes may use their own INI files
	if (!TFConfigUtils::LoadINISection(TEXT("InteractableConfig.ini"), SectionName, ConfigFilePath, LogTFInteraction, true))
	{
		return;
	}

	UE_LOG(LogTFInteraction, Log, TEXT("ATFInteractableActor: Loading config for InteractableID '%s'"), *SectionName);

#pragma region Interaction Settings

	GConfig->GetFloat(*SectionName, TEXT("InteractionDuration"), InteractionDuration, ConfigFilePath);
	GConfig->GetFloat(*SectionName, TEXT("MaxInteractionDistance"), MaxInteractionDistance, ConfigFilePath);
	GConfig->GetBool(*SectionName, TEXT("bCanInteract"), bCanInteract, ConfigFilePath);
	GConfig->GetBool(*SectionName, TEXT("bIsReusable"), bIsReusable, ConfigFilePath);
	GConfig->GetInt(*SectionName, TEXT("MaxUses"), MaxUses, ConfigFilePath);

	// Validate loaded values
	InteractionDuration = FMath::Clamp(InteractionDuration, 0.0f, 10.0f);
	MaxInteractionDistance = FMath::Clamp(MaxInteractionDistance, 50.0f, 1000.0f);

#pragma endregion Interaction Settings

#pragma region Icon Loading

	if (UTexture2D* LoadedIcon = TFConfigUtils::LoadAssetFromConfig<UTexture2D>(SectionName, TEXT("InteractionIcon"), ConfigFilePath, LogTFInteraction, TEXT("InteractionIcon")))
	{
		InteractionIcon = LoadedIcon;
	}

#pragma endregion Icon Loading

#pragma region Mesh Loading

	if (UStaticMesh* LoadedMesh = TFConfigUtils::LoadAssetFromConfig<UStaticMesh>(SectionName, TEXT("Mesh"), ConfigFilePath, LogTFInteraction, TEXT("Mesh")))
	{
		if (MeshComponent)
		{
			MeshComponent->SetStaticMesh(LoadedMesh);
		}
	}

#pragma endregion Mesh Loading

	UE_LOG(LogTFInteraction, Log, TEXT("ATFInteractableActor: Config loaded successfully for InteractableID '%s'"), *SectionName);
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

void ATFInteractableActor::ResetUses()
{
	CurrentUses = 0;
	bHasBeenUsed = false;
	bCanInteract = true;
}
