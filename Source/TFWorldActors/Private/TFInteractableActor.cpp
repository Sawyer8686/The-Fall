// Fill out your copyright notice in the Description page of Project Settings.

#include "TFInteractableActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
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
	FString ConfigFilePath = FPaths::ProjectConfigDir() / TEXT("InteractableConfig.ini");
	FConfigCacheIni::NormalizeConfigIniPath(ConfigFilePath);

	if (!FPaths::FileExists(ConfigFilePath))
	{
		// Silently return - derived classes may use their own INI files
		return;
	}

	const FString SectionName = InteractableID.ToString();

	FConfigFile ConfigFile;
	ConfigFile.Read(ConfigFilePath);

	if (!ConfigFile.Contains(SectionName))
	{
		// Silently return - derived classes may use their own INI files with different section names
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ATFInteractableActor: Loading config for InteractableID '%s'"), *SectionName);

	FString StringValue;

#pragma region Interaction Settings

	GConfig->GetFloat(*SectionName, TEXT("InteractionDuration"), InteractionDuration, ConfigFilePath);
	GConfig->GetFloat(*SectionName, TEXT("MaxInteractionDistance"), MaxInteractionDistance, ConfigFilePath);
	GConfig->GetBool(*SectionName, TEXT("bCanInteract"), bCanInteract, ConfigFilePath);
	GConfig->GetBool(*SectionName, TEXT("bIsReusable"), bIsReusable, ConfigFilePath);
	GConfig->GetInt(*SectionName, TEXT("MaxUses"), MaxUses, ConfigFilePath);

#pragma endregion Interaction Settings

#pragma region Icon Loading

	if (GConfig->GetString(*SectionName, TEXT("InteractionIcon"), StringValue, ConfigFilePath) && !StringValue.IsEmpty())
	{
		if (UTexture2D* LoadedIcon = LoadObject<UTexture2D>(nullptr, *StringValue))
		{
			InteractionIcon = LoadedIcon;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ATFInteractableActor: Failed to load InteractionIcon: %s"), *StringValue);
		}
	}

#pragma endregion Icon Loading

#pragma region Mesh Loading

	if (GConfig->GetString(*SectionName, TEXT("Mesh"), StringValue, ConfigFilePath) && !StringValue.IsEmpty())
	{
		if (UStaticMesh* LoadedMesh = LoadObject<UStaticMesh>(nullptr, *StringValue))
		{
			if (MeshComponent)
			{
				MeshComponent->SetStaticMesh(LoadedMesh);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ATFInteractableActor: Failed to load Mesh: %s"), *StringValue);
		}
	}

#pragma endregion Mesh Loading

	UE_LOG(LogTemp, Log, TEXT("ATFInteractableActor: Config loaded successfully for InteractableID '%s'"), *SectionName);
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
