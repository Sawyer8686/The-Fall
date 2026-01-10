#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "TFInteractable.h"
#include "TFWorldItemPickup.generated.h"

struct FTFItemsData;
class UStaticMeshComponent;

UCLASS()
class TFINVENTORY_API ATFWorldItemPickup : public AActor, public ITFInteractable
{
	GENERATED_BODY()

public:
	ATFWorldItemPickup();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Pickup")
	UStaticMeshComponent* MeshComp;

	// Row handle verso la DataTable che contiene FTFItemsData
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	FDataTableRowHandle ItemRow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (ClampMin = "1"))
	int32 Quantity = 1;

	// -1 => usa MaxDurability dell’item (se presente), o “nessuna durabilità”
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	float Durability = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (ClampMin = "50.0"))
	float InteractDistance = 300.0f;

	// Helper: risolve la row dalla DataTable
	bool ResolveItemData(FTFItemsData& OutData) const;

public:
	// ITFInteractable
	virtual FText GetInteractText_Implementation() const override;
	virtual bool Interact_Implementation(AActor* Interactor) override;
	virtual void OnConstruction(const FTransform& Transform) override;
};

