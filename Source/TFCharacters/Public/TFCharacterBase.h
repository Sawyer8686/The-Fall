// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TFCharacterBase.generated.h"


UCLASS(Abstract, NotBlueprintable)
class TFCHARACTERS_API ATFCharacterBase : public ACharacter
{
	GENERATED_BODY()

private:

#pragma region Movement

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsSneaking;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float SneakSpeed = 35.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
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

	UFUNCTION(BlueprintPure, Category = "Movement")
	FORCEINLINE bool IsSneaking() const { return bIsSneaking; }
};