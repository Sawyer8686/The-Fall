// Copyright TF Project. All Rights Reserved.

#include "TFPickupableActor.h"
#include "TFTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TFKeyHolderInterface.h"
#include "TFInventoryHolderInterface.h"
#include "Misc/ConfigCacheIni.h"


ATFPickupableActor::ATFPickupableActor()
{
	PrimaryActorTick.bCanEverTick = false;


	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(Root);
	AudioComponent->bAutoActivate = false;

	// Enable physics for pickable actors
	if (MeshComponent)
	{
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		MeshComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		MeshComponent->SetSimulatePhysics(true);
	}
}

void ATFPickupableActor::BeginPlay()
{
	Super::BeginPlay();
}

void ATFPickupableActor::LoadConfigFromINI()
{
	// First load base interactable config (InteractionDuration, MaxInteractionDistance, etc.)
	Super::LoadConfigFromINI();

	// Then load item-specific config
	if (InteractableID.IsNone())
	{
		return;
	}

	const FString SectionName = InteractableID.ToString();
	FString ConfigFilePath;

	if (!TFConfigUtils::LoadINISection(TEXT("ItemConfig.ini"), SectionName, ConfigFilePath, LogTFItem))
	{
		return;
	}

	ItemData.ItemID = InteractableID;

	UE_LOG(LogTFItem, Log, TEXT("ATFPickupableActor: Loading config for InteractableID '%s'"), *SectionName);

	FString StringValue;

#pragma region Item Type

	if (GConfig->GetString(*SectionName, TEXT("ItemType"), StringValue, ConfigFilePath))
	{
		static const TMap<FString, EItemType> ItemTypeMap = {
			{TEXT("Key"), EItemType::Key},
			{TEXT("Food"), EItemType::Food},
			{TEXT("Beverage"), EItemType::Beverage},
			{TEXT("Weapon"), EItemType::Weapon},
			{TEXT("Ammo"), EItemType::Ammo},
			{TEXT("Document"), EItemType::Document},
			{TEXT("Quest"), EItemType::Quest},
			{TEXT("Backpack"), EItemType::Backpack}
		};

		bool bMatched = false;
		ItemData.ItemType = TFConfigUtils::StringToEnum(StringValue, ItemTypeMap, EItemType::Key, &bMatched);
		if (!bMatched)
		{
			UE_LOG(LogTFItem, Warning, TEXT("ATFPickupableActor: Unknown ItemType '%s', defaulting to Key"), *StringValue);
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

	GConfig->GetFloat(*SectionName, TEXT("Weight"), ItemData.Weight, ConfigFilePath);
	GConfig->GetInt(*SectionName, TEXT("Value"), ItemData.Value, ConfigFilePath);

	ItemData.Weight = FMath::Max(0.0f, ItemData.Weight);
	ItemData.Value = FMath::Max(0, ItemData.Value);

#pragma endregion Basic Item Data

#pragma region Key-Specific Data

	if (ItemData.ItemType == EItemType::Key)
	{
		if (GConfig->GetString(*SectionName, TEXT("KeyID"), StringValue, ConfigFilePath) && !StringValue.IsEmpty())
		{
			ItemData.KeyID = FName(*StringValue);
		}
		else
		{
			ItemData.KeyID = InteractableID;
		}
	}

#pragma endregion Key-Specific Data

#pragma region Food/Beverage Data

	if (ItemData.ItemType == EItemType::Food || ItemData.ItemType == EItemType::Beverage)
	{
		GConfig->GetFloat(*SectionName, TEXT("HungerRestore"), ItemData.HungerRestore, ConfigFilePath);
		GConfig->GetFloat(*SectionName, TEXT("ThirstRestore"), ItemData.ThirstRestore, ConfigFilePath);

		ItemData.HungerRestore = FMath::Max(0.0f, ItemData.HungerRestore);
		ItemData.ThirstRestore = FMath::Max(0.0f, ItemData.ThirstRestore);
	}

#pragma endregion Food/Beverage Data

#pragma region Backpack-Specific Data

	if (ItemData.ItemType == EItemType::Backpack)
	{
		GConfig->GetInt(*SectionName, TEXT("BackpackSlots"), ItemData.BackpackSlots, ConfigFilePath);
		GConfig->GetFloat(*SectionName, TEXT("BackpackWeightLimit"), ItemData.BackpackWeightLimit, ConfigFilePath);

		ItemData.BackpackSlots = FMath::Max(1, ItemData.BackpackSlots);
		ItemData.BackpackWeightLimit = FMath::Max(1.0f, ItemData.BackpackWeightLimit);
	}

#pragma endregion Backpack-Specific Data

#pragma region Asset Loading

	ItemData.ItemIcon = TFConfigUtils::LoadAssetFromConfig<UTexture2D>(SectionName, TEXT("ItemIcon"), ConfigFilePath, LogTFItem, TEXT("ItemIcon"));

#pragma endregion Asset Loading

#pragma region Pickup Settings

	GConfig->GetBool(*SectionName, TEXT("bDestroyOnPickup"), bDestroyOnPickup, ConfigFilePath);
	GConfig->GetFloat(*SectionName, TEXT("DestroyDelay"), DestroyDelay, ConfigFilePath);

	// Validate pickup settings
	DestroyDelay = FMath::Max(0.0f, DestroyDelay);

#pragma endregion Pickup Settings

	UE_LOG(LogTFItem, Log, TEXT("ATFPickupableActor: Config loaded successfully for InteractableID '%s' (Type: %d)"),
		*SectionName, static_cast<int32>(ItemData.ItemType));
}

bool ATFPickupableActor::HandleKeyPickup(APawn* Picker)
{
	if (!Picker || ItemData.KeyID.IsNone())
	{
		UE_LOG(LogTFItem, Warning, TEXT("ATFPickupableActor: Key pickup failed - invalid Picker or KeyID"));
		return false;
	}

	ITFKeyHolderInterface* KeyHolder = Cast<ITFKeyHolderInterface>(Picker);
	if (!KeyHolder)
	{
		UE_LOG(LogTFItem, Warning, TEXT("ATFPickupableActor: Picker does not implement ITFKeyHolderInterface"));
		return false;
	}

	KeyHolder->AddKey(ItemData.KeyID, ItemData.ItemName);
	OnKeyCollected(Picker);

	UE_LOG(LogTFItem, Log, TEXT("ATFPickupableActor: Key '%s' added to key holder"), *ItemData.KeyID.ToString());
	return true;
}

bool ATFPickupableActor::HandleBackpackPickup(APawn* Picker)
{
	if (!Picker)
	{
		return false;
	}

	ITFInventoryHolderInterface* InventoryHolder = Cast<ITFInventoryHolderInterface>(Picker);
	if (!InventoryHolder)
	{
		UE_LOG(LogTFItem, Warning, TEXT("ATFPickupableActor: Picker does not implement ITFInventoryHolderInterface"));
		return false;
	}

	if (InventoryHolder->HasBackpack())
	{
		UE_LOG(LogTFItem, Warning, TEXT("ATFPickupableActor: Picker already has a backpack"));
		OnPickupFailed(Picker, FText::FromString("Already have a backpack"));
		return false;
	}

	if (!InventoryHolder->ActivateBackpack(ItemData.BackpackSlots, ItemData.BackpackWeightLimit))
	{
		UE_LOG(LogTFItem, Warning, TEXT("ATFPickupableActor: Failed to activate backpack"));
		return false;
	}

	InventoryHolder->SetPendingBackpackActor(this);

	// Disable physics before disabling collision to prevent the actor from falling
	if (MeshComponent && MeshComponent->IsSimulatingPhysics())
	{
		MeshComponent->SetSimulatePhysics(false);
	}
	SetActorEnableCollision(false);

	PlayPickupSound();

	UE_LOG(LogTFItem, Log, TEXT("ATFPickupableActor: Backpack confirm requested (Slots: %d, Weight: %.1f)"),
		ItemData.BackpackSlots, ItemData.BackpackWeightLimit);

	return false;
}

bool ATFPickupableActor::HandleInventoryPickup(APawn* Picker)
{
	if (!Picker)
	{
		return false;
	}

	ITFInventoryHolderInterface* InventoryHolder = Cast<ITFInventoryHolderInterface>(Picker);
	if (!InventoryHolder)
	{
		UE_LOG(LogTFItem, Warning, TEXT("ATFPickupableActor: Picker does not implement ITFInventoryHolderInterface"));
		return false;
	}

	if (!InventoryHolder->HasBackpack())
	{
		UE_LOG(LogTFItem, Warning, TEXT("ATFPickupableActor: Picker has no backpack"));
		OnPickupFailed(Picker, FText::FromString("No backpack equipped"));
		return false;
	}

	if (!InventoryHolder->HasSpaceForItem(ItemData))
	{
		FText Reason;
		if (InventoryHolder->GetFreeSlots() <= 0)
		{
			Reason = FText::FromString("Inventory full");
		}
		else
		{
			Reason = FText::FromString("Item too heavy");
		}
		UE_LOG(LogTFItem, Warning, TEXT("ATFPickupableActor: %s"), *Reason.ToString());
		OnPickupFailed(Picker, Reason);
		return false;
	}

	if (!InventoryHolder->AddItem(ItemData))
	{
		UE_LOG(LogTFItem, Warning, TEXT("ATFPickupableActor: Failed to add item to inventory"));
		return false;
	}

	return true;
}

void ATFPickupableActor::PlayPickupSound()
{
	if (PickupSound && AudioComponent)
	{
		AudioComponent->SetSound(PickupSound);
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

	if (ItemData.ItemType == EItemType::Backpack)
	{
		if (!HandleBackpackPickup(Picker))
		{
			return false;
		}
	}
	else if (ItemData.ItemType == EItemType::Key)
	{
		if (!HandleKeyPickup(Picker))
		{
			return false;
		}
	}
	else
	{
		if (!HandleInventoryPickup(Picker))
		{
			return false;
		}
	}

	PlayPickupSound();

	UE_LOG(LogTFItem, Log, TEXT("Picked up item: %s [Type: %d]"),
		*ItemData.ItemName.ToString(), static_cast<int32>(ItemData.ItemType));

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

	// Keys and backpacks can always be picked up
	if (ItemData.ItemType == EItemType::Key || ItemData.ItemType == EItemType::Backpack)
	{
		return true;
	}

	// All other items require a backpack
	ITFInventoryHolderInterface* InventoryHolder = Cast<ITFInventoryHolderInterface>(Picker);
	if (!InventoryHolder || !InventoryHolder->HasBackpack())
	{
		return false;
	}

	return true;
}

void ATFPickupableActor::OnPickupFailed(APawn* Picker, const FText& Reason)
{
	UE_LOG(LogTFItem, Warning, TEXT("Pickup failed: %s"), *Reason.ToString());

	OnItemPickupFailed(Picker, Reason);
}

bool ATFPickupableActor::ShouldDestroyOnPickup() const
{
	return bDestroyOnPickup;
}

void ATFPickupableActor::SetItemData(const FItemData& NewItemData)
{
	ItemData = NewItemData;

}

