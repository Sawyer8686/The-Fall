// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TFGameMode.generated.h"

class ATFDayNightCycle;

/**
 * TF Game Mode
 * Manages game rules, default classes, and global systems
 */
UCLASS()
class TF_API ATFGameMode : public AGameModeBase
{
	GENERATED_BODY()

#pragma region World Systems

protected:

	/** Cached reference to the Day/Night Cycle actor in the world */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Systems")
	ATFDayNightCycle* DayNightCycle;

	/** Whether to automatically find the Day/Night Cycle actor on BeginPlay */
	UPROPERTY(EditDefaultsOnly, Category = "World Systems")
	bool bAutoFindDayNightCycle = true;

#pragma endregion World Systems

protected:

	virtual void BeginPlay() override;

	/** Find and cache the Day/Night Cycle actor in the world */
	void FindDayNightCycle();

public:

	ATFGameMode();

#pragma region Accessors

	/** Get the Day/Night Cycle actor */
	UFUNCTION(BlueprintCallable, Category = "World Systems")
	ATFDayNightCycle* GetDayNightCycle() const { return DayNightCycle; }

	/** Set the Day/Night Cycle actor manually */
	UFUNCTION(BlueprintCallable, Category = "World Systems")
	void SetDayNightCycle(ATFDayNightCycle* NewDayNightCycle);

#pragma endregion Accessors
};
