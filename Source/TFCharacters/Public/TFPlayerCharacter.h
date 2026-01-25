// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TFCharacterBase.h"
#include "TFKeyHolderInterface.h"
#include "TFInventoryHolderInterface.h"
#include "TFPlayerCharacter.generated.h"

class UCameraComponent;
class UTFStaminaComponent;
class UTFStatsComponent;
class UTFInteractionComponent;
class UTFInventoryComponent;

UENUM()
enum class ESprintBlockReason : uint8
{
	Sneaking,
	NoStamina
};

DECLARE_MULTICAST_DELEGATE(FOnKeyCollectionChanged);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCharacterBackpackEquipRequested, int32, float);

UCLASS()
class TFCHARACTERS_API ATFPlayerCharacter : public ATFCharacterBase, public ITFKeyHolderInterface, public ITFInventoryHolderInterface
{
	GENERATED_BODY()

private:

#pragma region Components

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UTFStaminaComponent* StaminaComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UTFStatsComponent* StatsComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UTFInteractionComponent* InteractionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UTFInventoryComponent* InventoryComponent;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* FirstPersonCamera;

#pragma endregion Components

#pragma region Movement & Sprint

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	bool bIsSprinting;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Stamina", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float ExhaustedSpeedMultiplier = 0.7f;

#pragma endregion Movement & Sprint

#pragma region Camera Settings

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FVector FirstPersonCameraOffset = FVector(15.0f, 20.0f, 2.5f);

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FRotator FirstPersonCameraRotation = FRotator(0.0f, -90.0f, 90.0f);

#pragma endregion Camera Settings

protected:

#pragma region Sprint & Stamina

	void SetSprinting(const bool bSprinting);
	float GetSprintSpeed() const;
	virtual void UpdateMovementSpeed() override;

#pragma endregion Sprint & Stamina

#pragma region Stamina Events

	void BindStaminaEvents();
	void UnbindStaminaEvents();
	void HandleStaminaDepleted();
	void HandleStaminaRecovered();
	virtual void OnStaminaDepleted();
	virtual void OnStaminaRecovered();
	virtual void OnSprintBlocked(ESprintBlockReason Reason) {}

#pragma endregion Stamina Events

#pragma region Overrides

	virtual bool CanCharacterJump() const override;
	virtual void HasJumped() override;

#pragma endregion Overrides

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

	ATFPlayerCharacter();

#pragma region Accessors

	UFUNCTION(BlueprintCallable, Category = "Components")
	UTFStaminaComponent* GetStaminaComponent() const { return StaminaComponent; }

	UFUNCTION(BlueprintCallable, Category = "Components")
	UTFStatsComponent* GetStatsComponent() const { return StatsComponent; }

	UFUNCTION(BlueprintCallable, Category = "Components")
	UTFInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

	UFUNCTION(BlueprintCallable, Category = "Components")
	UTFInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	UFUNCTION(BlueprintCallable, Category = "Camera")
	UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsSprinting() const { return bIsSprinting; }

#pragma endregion Accessors

#pragma region Movement API (Called by PlayerController)

	/** Start sprinting - called by PlayerController */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartSprinting();

	/** Stop sprinting - called by PlayerController */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopSprinting();

	/** Start sneaking - called by PlayerController */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartSneaking();

	/** Stop sneaking - called by PlayerController */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopSneaking();

	/** Try to jump - called by PlayerController */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void TryJump();

#pragma endregion Movement API

#pragma region Key Collection

protected:

	UPROPERTY(VisibleAnywhere, Category = "Keys")
	TMap<FName, FText> CollectedKeys;

public:

	FOnKeyCollectionChanged OnKeyCollectionChanged;

	/** Called when backpack equip confirmation is requested */
	FOnCharacterBackpackEquipRequested OnBackpackEquipRequested;

	virtual bool HasKey(FName KeyID) const override;
	virtual void AddKey(FName KeyID, const FText& KeyName = FText::GetEmpty()) override;
	virtual bool RemoveKey(FName KeyID) override;
	virtual void OnKeyAdded(FName KeyID) {}
	virtual void OnKeyRemoved(FName KeyID) {}

	const TMap<FName, FText>& GetCollectedKeys() const { return CollectedKeys; }

#pragma endregion Key Collection

#pragma region Inventory

public:

	virtual bool HasBackpack() const override;
	virtual bool ActivateBackpack(int32 Slots, float WeightLimit) override;
	virtual void SetPendingBackpackActor(AActor* Actor) override;
	virtual bool AddItem(const FItemData& Item) override;
	virtual bool RemoveItem(FName ItemID) override;
	virtual bool HasItem(FName ItemID) const override;
	virtual bool HasSpaceForItem(const FItemData& Item) const override;
	virtual bool CanCarryWeight(float AdditionalWeight) const override;
	virtual int32 GetFreeSlots() const override;
	virtual float GetRemainingCapacity() const override;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool DropItem(FName ItemID);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool DropBackpack();

	/** Confirm backpack equip - called by PlayerController */
	void ConfirmBackpackEquip();

	/** Cancel backpack equip - called by PlayerController */
	void CancelBackpackEquip();

private:

	int32 PendingBackpackSlots = 0;
	float PendingBackpackWeightLimit = 0.0f;
	TWeakObjectPtr<AActor> PendingBackpackActor;
	FItemData EquippedBackpackData;

#pragma endregion Inventory
};
