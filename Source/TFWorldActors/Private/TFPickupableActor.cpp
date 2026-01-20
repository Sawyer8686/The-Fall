// Fill out your copyright notice in the Description page of Project Settings.

#include "TFPickupableActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TFKeyHolderInterface.h"
#include "Misc/ConfigCacheIni.h"


ATFPickupableActor::ATFPickupableActor()
{
	PrimaryActorTick.bCanEverTick = false;
	InteractionDuration = 0.0f;

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(Root);
	AudioComponent->bAutoActivate = false;
}

void ATFPickupableActor::BeginPlay()
{
	// Use ItemID as InteractableID if not explicitly set
	if (InteractableID.IsNone() && !ItemID.IsNone())
	{
		InteractableID = ItemID;
	}

	// Call parent BeginPlay which handles base data-driven config
	Super::BeginPlay();

	// Apply item mesh after all config is loaded
	if (ItemData.ItemMesh && MeshComponent)
	{
		MeshComponent->SetStaticMesh(ItemData.ItemMesh);
	}
}

void ATFPickupableActor::LoadConfigFromINI()
{
	// First load base interactable config (InteractionDuration, MaxInteractionDistance, etc.)
	Super::LoadConfigFromINI();

	// Then load item-specific config
	if (ItemID.IsNone())
	{
		return;
	}

	FString ConfigFilePath = FPaths::ProjectConfigDir() / TEXT("ItemConfig.ini");
	FConfigCacheIni::NormalizeConfigIniPath(ConfigFilePath);

	if (!FPaths::FileExists(ConfigFilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("ATFPickupableActor: ItemConfig.ini not found at %s"), *ConfigFilePath);
		return;
	}

	const FString SectionName = ItemID.ToString();

	FConfigFile ConfigFile;
	ConfigFile.Read(ConfigFilePath);

	if (!ConfigFile.Contains(SectionName))
	{
		UE_LOG(LogTemp, Warning, TEXT("ATFPickupableActor: Section [%s] not found in ItemConfig.ini"), *SectionName);
		return;
	}

	ItemData.ItemID = ItemID;

	UE_LOG(LogTemp, Log, TEXT("ATFPickupableActor: Loading config for ItemID '%s'"), *SectionName);

	FString StringValue;

#pragma region Item Type

	if (GConfig->GetString(*SectionName, TEXT("ItemType"), StringValue, ConfigFilePath))
	{
		if (StringValue.Equals(TEXT("Generic"), ESearchCase::IgnoreCase))
		{
			ItemData.ItemType = EItemType::Generic;
		}
		else if (StringValue.Equals(TEXT("Key"), ESearchCase::IgnoreCase))
		{
			ItemData.ItemType = EItemType::Key;
		}
		else if (StringValue.Equals(TEXT("Consumable"), ESearchCase::IgnoreCase))
		{
			ItemData.ItemType = EItemType::Consumable;
		}
		else if (StringValue.Equals(TEXT("Weapon"), ESearchCase::IgnoreCase))
		{
			ItemData.ItemType = EItemType::Weapon;
		}
		else if (StringValue.Equals(TEXT("Ammo"), ESearchCase::IgnoreCase))
		{
			ItemData.ItemType = EItemType::Ammo;
		}
		else if (StringValue.Equals(TEXT("Document"), ESearchCase::IgnoreCase))
		{
			ItemData.ItemType = EItemType::Document;
		}
		else if (StringValue.Equals(TEXT("Quest"), ESearchCase::IgnoreCase))
		{
			ItemData.ItemType = EItemType::Quest;
		}
	}

#pragma endregion Item Type

#pragma region Basic Item Data

	if (GConfig->GetString(*SectionName, TEXT("ItemName"), StringValue, ConfigFilePath))
	{
		ItemData.ItemName = FText::FromString(StringValue);
	}

	if (GConfig->GetString(*SectionName, TEXT("ItemDescription"), StringValue, ConfigFilePath))
	{
		ItemData.ItemDescription = FText::FromString(StringValue);
	}

	GConfig->GetInt(*SectionName, TEXT("Quantity"), ItemData.Quantity, ConfigFilePath);
	GConfig->GetBool(*SectionName, TEXT("bIsStackable"), ItemData.bIsStackable, ConfigFilePath);
	GConfig->GetInt(*SectionName, TEXT("MaxStackSize"), ItemData.MaxStackSize, ConfigFilePath);
	GConfig->GetFloat(*SectionName, TEXT("Weight"), ItemData.Weight, ConfigFilePath);
	GConfig->GetInt(*SectionName, TEXT("Value"), ItemData.Value, ConfigFilePath);

#pragma endregion Basic Item Data

#pragma region Key-Specific Data

	if (ItemData.ItemType == EItemType::Key)
	{
		if (GConfig->GetString(*SectionName, TEXT("KeyID"), StringValue, ConfigFilePath))
		{
			ItemData.KeyID = FName(*StringValue);
		}
		else
		{
			ItemData.KeyID = ItemID;
		}
	}

#pragma endregion Key-Specific Data

#pragma region Mesh Loading

	if (GConfig->GetString(*SectionName, TEXT("ItemMesh"), StringValue, ConfigFilePath) && !StringValue.IsEmpty())
	{
		if (UStaticMesh* LoadedMesh = LoadObject<UStaticMesh>(nullptr, *StringValue))
		{
			ItemData.ItemMesh = LoadedMesh;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ATFPickupableActor: Failed to load ItemMesh: %s"), *StringValue);
		}
	}

#pragma endregion Mesh Loading

#pragma region Icon Loading

	if (GConfig->GetString(*SectionName, TEXT("ItemIcon"), StringValue, ConfigFilePath) && !StringValue.IsEmpty())
	{
		if (UTexture2D* LoadedIcon = LoadObject<UTexture2D>(nullptr, *StringValue))
		{
			ItemData.ItemIcon = LoadedIcon;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ATFPickupableActor: Failed to load ItemIcon: %s"), *StringValue);
		}
	}

#pragma endregion Icon Loading

#pragma region Audio Loading

	if (GConfig->GetString(*SectionName, TEXT("PickupSound"), StringValue, ConfigFilePath) && !StringValue.IsEmpty())
	{
		ItemData.PickupSound = LoadObject<USoundBase>(nullptr, *StringValue);
		if (!ItemData.PickupSound)
		{
			UE_LOG(LogTemp, Warning, TEXT("ATFPickupableActor: Failed to load PickupSound: %s"), *StringValue);
		}
	}

#pragma endregion Audio Loading

#pragma region Pickup Settings

	GConfig->GetBool(*SectionName, TEXT("bDestroyOnPickup"), bDestroyOnPickup, ConfigFilePath);
	GConfig->GetFloat(*SectionName, TEXT("DestroyDelay"), DestroyDelay, ConfigFilePath);

#pragma endregion Pickup Settings

	UE_LOG(LogTemp, Log, TEXT("ATFPickupableActor: Config loaded successfully for ItemID '%s' (Type: %d)"),
		*SectionName, static_cast<int32>(ItemData.ItemType));
}

bool ATFPickupableActor::HandleKeyPickup(APawn* Picker)
{
	if (!Picker || ItemData.KeyID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("ATFPickupableActor: Key pickup failed - invalid Picker or KeyID"));
		return false;
	}

	ITFKeyHolderInterface* KeyHolder = Cast<ITFKeyHolderInterface>(Picker);
	if (!KeyHolder)
	{
		UE_LOG(LogTemp, Warning, TEXT("ATFPickupableActor: Picker does not implement ITFKeyHolderInterface"));
		return false;
	}

	KeyHolder->AddKey(ItemData.KeyID);
	OnKeyCollected(Picker);

	UE_LOG(LogTemp, Log, TEXT("ATFPickupableActor: Key '%s' added to key holder"), *ItemData.KeyID.ToString());
	return true;
}

void ATFPickupableActor::PlayPickupSound()
{
	if (ItemData.PickupSound && AudioComponent)
	{
		AudioComponent->SetSound(ItemData.PickupSound);
		AudioComponent->Play();
	}
}

bool ATFPickupableActor::Interact(APawn* InstigatorPawn)
{
	bool bSuccess = OnPickup(InstigatorPawn);

	if (bSuccess)
	{
		OnItemPickedUp(InstigatorPawn);

		if (bDestroyOnPickup)
		{
			if (DestroyDelay > 0.0f)
			{
				if (MeshComponent)
				{
					MeshComponent->SetVisibility(false);
				}

				SetActorEnableCollision(false);

				SetLifeSpan(DestroyDelay);
			}
			else
			{
				Destroy();
			}
		}

		return true;
	}

	return false;
}

FInteractionData ATFPickupableActor::GetInteractionData(APawn* InstigatorPawn) const
{
	FInteractionData Data = Super::GetInteractionData(InstigatorPawn);
	return Data;
}

bool ATFPickupableActor::OnPickup(APawn* Picker)
{
	if (!CanPickup(Picker))
	{
		OnPickupFailed(Picker, FText::FromString("Cannot pickup item"));
		return false;
	}

	if (ItemData.ItemType == EItemType::Key)
	{
		if (!HandleKeyPickup(Picker))
		{
			return false;
		}
	}

	PlayPickupSound();

	UE_LOG(LogTemp, Log, TEXT("Picked up item: %s (x%d) [Type: %d]"),
		*ItemData.ItemName.ToString(), ItemData.Quantity, static_cast<int32>(ItemData.ItemType));

	return true;
}

FItemData ATFPickupableActor::GetItemData() const
{
	return ItemData;
}

bool ATFPickupableActor::CanPickup(APawn* Picker) const
{
	if (!Picker)
	{
		return false;
	}

	return true;
}

void ATFPickupableActor::OnPickupFailed(APawn* Picker, const FText& Reason)
{
	UE_LOG(LogTemp, Warning, TEXT("Pickup failed: %s"), *Reason.ToString());

	OnItemPickupFailed(Picker, Reason);
}

bool ATFPickupableActor::ShouldDestroyOnPickup() const
{
	return bDestroyOnPickup;
}

void ATFPickupableActor::SetItemData(const FItemData& NewItemData)
{
	ItemData = NewItemData;

	if (ItemData.ItemMesh && MeshComponent)
	{
		MeshComponent->SetStaticMesh(ItemData.ItemMesh);
	}
}

void ATFPickupableActor::SetQuantity(int32 NewQuantity)
{
	ItemData.Quantity = FMath::Max(1, NewQuantity);
}
