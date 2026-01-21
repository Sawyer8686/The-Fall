// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TFCharacterBase.generated.h"


UCLASS(Abstract)
class TFCHARACTERS_API ATFCharacterBase : public ACharacter
{
	GENERATED_BODY()

private:

#pragma region Movement

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	bool bIsSneaking;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SneakSpeed = 35.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float WalkSpeed = 150.f;

#pragma endregion Movement

protected:

	virtual bool CanCharacterJump() const;
	virtual void HasJumped();
	float GetSneakSpeed() const;
	float GetWalkSpeed() const;
	void SetSneaking(const bool bSneaking);
	virtual void UpdateMovementSpeed();

public:

	ATFCharacterBase();

	FORCEINLINE bool IsSneaking() const { return bIsSneaking; }
};