// Copyright TF Project. All Rights Reserved.

#include "TFBaseContainerActor.h"
#include "TFTypes.h"
#include "Blueprint/UserWidget.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
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

void ATFBaseContainerActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// If this container is destroyed while open, clean up
	if (FTFContainerContext::ActiveContainer == this)
	{
		CloseContainer();
	}

	Super::EndPlay(EndPlayReason);
}

void ATFBaseContainerActor::LoadConfigFromINI()
{
	Super::LoadConfigFromINI();

	if (InteractableID.IsNone())
	{
		return;
	}

	const FString SectionName = InteractableID.ToString();
	FString ConfigFilePath;

	if (!TFConfigUtils::LoadINISection(TEXT("ContainerConfig.ini"), SectionName, ConfigFilePath, LogTFContainer))
	{
		return;
	}

	UE_LOG(LogTFContainer, Log, TEXT("ATFBaseContainerActor: Loading container config for '%s'"), *SectionName);

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

	// Stop all movement immediately
	if (ACharacter* Character = Cast<ACharacter>(InstigatorPawn))
	{
		if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
		{
			MovementComp->StopMovementImmediately();
			MovementComp->Velocity = FVector::ZeroVector;
		}
	}

	FTFContainerContext::ActiveContainer = this;

	ActiveWidget = CreateWidget<UUserWidget>(PC, ContainerWidgetClass);
	if (!ActiveWidget)
	{
		FTFContainerContext::ActiveContainer = nullptr;
		return;
	}

	ActiveWidget->AddToViewport(10);

	PC->bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);
	PC->SetIgnoreMoveInput(true);
	PC->SetIgnoreLookInput(true);

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
		PC->ResetIgnoreMoveInput();
		PC->ResetIgnoreLookInput();
	}

	UE_LOG(LogTFContainer, Log, TEXT("ATFBaseContainerActor: Container closed for '%s'"), *ContainerDisplayName.ToString());
}
