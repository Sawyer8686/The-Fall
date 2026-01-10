#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TFInteractable.generated.h"

UINTERFACE(BlueprintType)
class TFCOREDATA_API UTFInteractable : public UInterface
{
	GENERATED_BODY()
};

class TFCOREDATA_API ITFInteractable
{
	GENERATED_BODY()

public:
	// Testo per UI (opzionale)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractText() const;

	// Esegui interazione
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool Interact(AActor* Interactor);
};
