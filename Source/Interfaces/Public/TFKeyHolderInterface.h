
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TFKeyHolderInterface.generated.h"

UINTERFACE(MinimalAPI)
class UTFKeyHolderInterface : public UInterface
{
	GENERATED_BODY()
};

class INTERFACES_API ITFKeyHolderInterface
{
	GENERATED_BODY()

public:

	virtual bool HasKey(FName KeyID) const { return false; }

	virtual void AddKey(FName KeyID, const FText& KeyName = FText::GetEmpty()) {}

	virtual bool RemoveKey(FName KeyID) { return false; }
};
