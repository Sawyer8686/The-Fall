#include "TFWorldItemPickup.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"

// Dati item
#include "TFItemsData.h"

// Inventory component
#include "TFInventoryComponent.h"

ATFWorldItemPickup::ATFWorldItemPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);

	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComp->SetCollisionObjectType(ECC_WorldDynamic);
	MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
}

void ATFWorldItemPickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	FTFItemsData ItemData;
	if (!ResolveItemData(ItemData))
	{
		// Pulisci mesh se row invalida
		if (MeshComp)
		{
			MeshComp->SetStaticMesh(nullptr);
		}
		return;
	}

	// Applica static mesh (solo se presente)
	if (MeshComp)
	{
		MeshComp->SetStaticMesh(ItemData.Mesh);
	}
}

bool ATFWorldItemPickup::ResolveItemData(FTFItemsData& OutData) const
{
	if (!ItemRow.DataTable || ItemRow.RowName.IsNone())
	{
		return false;
	}

	const FTFItemsData* Row = ItemRow.DataTable->FindRow<FTFItemsData>(ItemRow.RowName, TEXT("TFWorldItemPickup"));
	if (!Row)
	{
		return false;
	}

	OutData = *Row;
	return OutData.IsValid();
}

FText ATFWorldItemPickup::GetInteractText_Implementation() const
{
	// In mancanza di dati risolti, fallback generico
	if (!ItemRow.DataTable || ItemRow.RowName.IsNone())
	{
		return FText::FromString(TEXT("Raccogli"));
	}

	return FText::FromString(TEXT("Raccogli"));
}

bool ATFWorldItemPickup::Interact_Implementation(AActor* Interactor)
{
	if (!Interactor || Quantity <= 0)
	{
		return false;
	}

	FTFItemsData ItemData;
	if (!ResolveItemData(ItemData))
	{
		UE_LOG(LogTemp, Warning, TEXT("Pickup Interact: ItemRow invalid or not found."));
		return false;
	}

	// Cerca InventoryComponent sull’interactor o sul Pawn/Character
	UTFInventoryComponent* InvComp = Interactor->FindComponentByClass<UTFInventoryComponent>();
	if (!InvComp)
	{
		APawn* Pawn = Cast<APawn>(Interactor);
		if (Pawn)
		{
			InvComp = Pawn->FindComponentByClass<UTFInventoryComponent>();
		}
	}

	if (!InvComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pickup Interact: InventoryComponent not found on interactor."));
		return false;
	}

	// Durability: se non specificata, lascia -1 (la tua logica deciderà)
	const float UseDurability = Durability;

	// AddItemToInventory ritorna “quantità non aggiunta”
	const int32 NotAdded = InvComp->AddItemToInventory(ItemData, Quantity, UseDurability);
	const int32 Added = Quantity - NotAdded;

	if (Added <= 0)
	{
		// inventario pieno o item invalid
		return false;
	}

	// Se aggiunta parziale, riduci quantità e resta nel mondo; se totale distruggi
	if (NotAdded > 0)
	{
		Quantity = NotAdded;
		return true;
	}

	Destroy();
	return true;
}

