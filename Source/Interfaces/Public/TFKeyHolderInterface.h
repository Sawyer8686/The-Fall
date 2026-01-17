
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TFKeyHolderInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UTFKeyHolderInterface : public UInterface
{
	GENERATED_BODY()
};

class INTERFACES_API ITFKeyHolderInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Keys")
	bool HasKey(FName KeyID) const;
	virtual bool HasKey_Implementation(FName KeyID) const { return false; }

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Keys")
	void AddKey(FName KeyID);
	virtual void AddKey_Implementation(FName KeyID) {}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Keys")
	bool RemoveKey(FName KeyID);
	virtual bool RemoveKey_Implementation(FName KeyID) { return false; }
};
