// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TFCharacterBase.generated.h"

/**
 * Base Character Class
 * Provides fundamental movement capabilities for all characters (Player, NPCs, Animals)
 * Does NOT include stamina - that's player-specific functionality
 */
UCLASS(Abstract, NotBlueprintable)
class TFCHARACTERS_API ATFCharacterBase : public ACharacter
{
	GENERATED_BODY()

private:

#pragma region Movement

	/** Is character currently sneaking */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsSneaking;

	/** Sneak movement speed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float SneakSpeed = 35.f;

	/** Normal walk movement speed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float WalkSpeed = 150.f;

#pragma endregion Movement

protected:

	virtual void BeginPlay() override;

	/** Check if character can jump (base check, override for stamina/conditions) */
	virtual bool CanCharacterJump() const;

	/** Execute jump (override to add stamina consumption) */
	virtual void HasJumped();

	/** Get current sneak speed */
	float GetSneakSpeed() const;

	/** Get current walk speed */
	float GetWalkSpeed() const;

	/** Set sneaking state */
	void SetSneaking(const bool bSneaking);

	/** Update character movement speed */
	virtual void UpdateMovementSpeed();

public:

	ATFCharacterBase();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Check if currently sneaking */
	UFUNCTION(BlueprintPure, Category = "Movement")
	FORCEINLINE bool IsSneaking() const { return bIsSneaking; }
};