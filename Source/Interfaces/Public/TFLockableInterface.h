
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
	virtual bool IsCurrentlyLocked() const { return false; }
	virtual bool CanToggleLock(APawn* Character) const { return false; }

	/** Returns the time at which the key will break during unlock, or -1 if it won't break */
	virtual float CalculateKeyBreakTime() const { return -1.0f; }

	/** Force the key to break (called by controller when break timer fires) */
	virtual void ForceKeyBreak(APawn* Character) {}
};
