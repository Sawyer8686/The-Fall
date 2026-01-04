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
	bool bIsSprinting;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsSneaking;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float SneakSpeed = 35.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float WalkSpeed = 150.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float SprintSpeed = 300.f;

#pragma endregion Movement


protected:

	virtual void BeginPlay() override;
	bool CanCharacterJump() const;
	void HasJumped();
	float GetSneakSpeed() const;
	float GetWalkSpeed() const;
	float GetSprintSpeed() const;

	void SetSprinting(const bool bSprinting);
	void SetSneaking(const bool bSneaking);



public:	

	ATFCharacterBase();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
