#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FInventorySlot.h"
#include "TFInventoryComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TFINVENTORY_API UTFInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

private:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int TotalNumberOfSlots = 10;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FInventorySlot> InventoryContents;

protected:

	virtual void BeginPlay() override;

	virtual void ResizeInventory();

public:

	UTFInventoryComponent();

	bool UpdateInventorySlots(const int NewSlots);
};
