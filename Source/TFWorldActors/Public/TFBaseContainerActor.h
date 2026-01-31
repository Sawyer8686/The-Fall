// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TFInteractableActor.h"
#include "TFContainerInterface.h"
#include "TFBaseContainerActor.generated.h"

class UUserWidget;

UCLASS()
class TFWORLDACTORS_API ATFBaseContainerActor : public ATFInteractableActor, public ITFContainerInterface
{
	GENERATED_BODY()

protected:

#pragma region Container Config

	UPROPERTY(VisibleAnywhere, Category = "Container")
	int32 MaxCapacity = 10;

	UPROPERTY(VisibleAnywhere, Category = "Container")
	FText ContainerDisplayName;

#pragma endregion Container Config

#pragma region Container State

	UPROPERTY(VisibleAnywhere, Category = "Container|Items")
	TArray<FItemData> ContainerItems;

#pragma endregion Container State

#pragma region Widget

	UPROPERTY(EditAnywhere, Category = "Container|Widget")
	TSubclassOf<UUserWidget> ContainerWidgetClass;

	UPROPERTY()
	UUserWidget* ActiveWidget;

#pragma endregion Widget

#pragma region Delegates

	FOnContainerContentChanged OnContainerContentChanged;

#pragma endregion Delegates

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void LoadConfigFromINI() override;

public:

	ATFBaseContainerActor();

#pragma region Interaction

	virtual void OnInteracted(APawn* InstigatorPawn) override;

#pragma endregion Interaction

#pragma region ITFContainerInterface

	virtual const TArray<FItemData>& GetContainerItems() const override { return ContainerItems; }
	virtual int32 GetMaxCapacity() const override { return MaxCapacity; }
	virtual int32 GetContainerUsedSlots() const override { return ContainerItems.Num(); }
	virtual int32 GetContainerFreeSlots() const override;
	virtual bool ContainerHasSpace() const override;
	virtual bool AddItemToContainer(const FItemData& Item) override;
	virtual bool RemoveItemFromContainer(FName ItemID) override;
	virtual const FItemData* GetContainerItem(FName ItemID) const override;
	virtual FText GetContainerName() const override { return ContainerDisplayName; }
	virtual void CloseContainer() override;
	virtual FOnContainerContentChanged& GetOnContainerChanged() override { return OnContainerContentChanged; }

#pragma endregion ITFContainerInterface
};
