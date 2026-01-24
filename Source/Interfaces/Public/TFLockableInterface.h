
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TFLockableInterface.generated.h"

UINTERFACE(MinimalAPI)
class UTFLockableInterface : public UInterface
{
	GENERATED_BODY()
};

class INTERFACES_API ITFLockableInterface
{
	GENERATED_BODY()

public:

	virtual bool ToggleLock(APawn* Character) { return false; }
	virtual float GetLockDuration() const { return 0.0f; }
};
