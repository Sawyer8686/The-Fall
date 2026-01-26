// Copyright TF Project. All Rights Reserved.

#include "TFBaseContainerActor.h"
#include "TFTypes.h"
#include "TFContainerWidget.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"

ATFBaseContainerActor::ATFBaseContainerActor()
{
	ContainerDisplayName = FText::FromString(TEXT("Contenitore"));
}

void ATFBaseContainerActor::BeginPlay()
{
	Super::BeginPlay();
}

void ATFBaseContainerActor::LoadConfigFromINI()
{
	const FString SectionName = InteractableID.ToString();
	FString ConfigFilePath;

	if (!TFConfigUtils::LoadINISection(TEXT("ContainerConfig.ini"), SectionName, ConfigFilePath, LogTFContainer, true))
	{
		return;
	}

	UE_LOG(LogTFContainer, Log, TEXT("ATFBaseContainerActor: Loading container config for '%s'"), *SectionName);

#pragma region Interaction Settings

	GConfig->GetFloat(*SectionName, TEXT("MaxInteractionDistance"), MaxInteractionDistance, ConfigFilePath);
	GConfig->GetBool(*SectionName, TEXT("bCanInteract"), bCanInteract, ConfigFilePath);
	MaxInteractionDistance = FMath::Clamp(MaxInteractionDistance, 50.0f, 1000.0f);

#pragma endregion Interaction Settings

#pragma region Mesh Loading

	if (UStaticMesh* LoadedMesh = TFConfigUtils::LoadAssetFromConfig<UStaticMesh>(SectionName, TEXT("Mesh"), ConfigFilePath, LogTFContainer, TEXT("Mesh")))
	{
		if (MeshComponent)
		{
			MeshComponent->SetStaticMesh(LoadedMesh);
		}
	}

#pragma endregion Mesh Loading

#pragma region Container Settings

	GConfig->GetInt(*SectionName, TEXT("MaxCapacity"), MaxCapacity, ConfigFilePath);
	MaxCapacity = FMath::Max(1, MaxCapacity);

	FString ContainerNameStr;
	if (GConfig->GetString(*SectionName, TEXT("ContainerName"), ContainerNameStr, ConfigFilePath) && !ContainerNameStr.IsEmpty())
	{
		ContainerDisplayName = FText::FromString(ContainerNameStr);
	}

#pragma endregion Container Settings

	UE_LOG(LogTFContainer, Log, TEXT("ATFBaseContainerActor: Config loaded (MaxCapacity: %d, Name: '%s')"), MaxCapacity, *ContainerDisplayName.ToString());
}

void ATFBaseContainerActor::OnInteracted(APawn* InstigatorPawn)
{
	if (!InstigatorPawn || !ContainerWidgetClass)
	{
		return;
	}

	if (ActiveWidget)
	{
		CloseContainer();
		return;
	}

	APlayerController* PC = Cast<APlayerController>(InstigatorPawn->GetController());
	if (!PC)
	{
		return;
	}

	// Stop sprinting when opening container (using reflection to avoid circular dependency)
	if (UFunction* StopSprintingFunc = InstigatorPawn->FindFunction(FName("StopSprinting")))
	{
		InstigatorPawn->ProcessEvent(StopSprintingFunc, nullptr);
	}

	FTFContainerContext::ActiveContainer = this;

	ActiveWidget = CreateWidget<UTFContainerWidget>(PC, ContainerWidgetClass);
	if (!ActiveWidget)
	{
		FTFContainerContext::ActiveContainer = nullptr;
		return;
	}

	ActiveWidget->SetContainerSource(this);
	ActiveWidget->AddToViewport(10);

	PC->bShowMouseCursor = true;
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);

	UE_LOG(LogTFContainer, Log, TEXT("ATFBaseContainerActor: Container widget opened for '%s'"), *ContainerDisplayName.ToString());
}

bool ATFBaseContainerActor::AddItemToContainer(const FItemData& Item)
{
	if (!ContainerHasSpace())
	{
		UE_LOG(LogTFContainer, Warning, TEXT("ATFBaseContainerActor: Cannot add item '%s' - container full (%d/%d)"),
			*Item.ItemName.ToString(), GetContainerUsedSlots(), MaxCapacity);
		return false;
	}

	ContainerItems.Add(Item);

	UE_LOG(LogTFContainer, Log, TEXT("ATFBaseContainerActor: Added item '%s' (%d/%d slots used)"),
		*Item.ItemName.ToString(), GetContainerUsedSlots(), MaxCapacity);

	OnContainerContentChanged.Broadcast();

	return true;
}

bool ATFBaseContainerActor::RemoveItemFromContainer(FName ItemID)
{
	if (ItemID.IsNone())
	{
		return false;
	}

	for (int32 i = ContainerItems.Num() - 1; i >= 0; --i)
	{
		if (ContainerItems[i].ItemID == ItemID)
		{
			ContainerItems.RemoveAt(i);

			UE_LOG(LogTFContainer, Log, TEXT("ATFBaseContainerActor: Removed item '%s' (%d/%d slots used)"),
				*ItemID.ToString(), GetContainerUsedSlots(), MaxCapacity);

			OnContainerContentChanged.Broadcast();
			return true;
		}
	}

	return false;
}

const FItemData* ATFBaseContainerActor::GetContainerItem(FName ItemID) const
{
	for (const FItemData& Item : ContainerItems)
	{
		if (Item.ItemID == ItemID)
		{
			return &Item;
		}
	}
	return nullptr;
}

bool ATFBaseContainerActor::ContainerHasSpace() const
{
	return GetContainerUsedSlots() < MaxCapacity;
}

int32 ATFBaseContainerActor::GetContainerFreeSlots() const
{
	return FMath::Max(0, MaxCapacity - GetContainerUsedSlots());
}

void ATFBaseContainerActor::CloseContainer()
{
	if (ActiveWidget)
	{
		ActiveWidget->RemoveFromParent();
		ActiveWidget = nullptr;
	}

	FTFContainerContext::ActiveContainer = nullptr;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		PC->bShowMouseCursor = false;
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
	}

	UE_LOG(LogTFContainer, Log, TEXT("ATFBaseContainerActor: Container widget closed for '%s'"), *ContainerDisplayName.ToString());
}
