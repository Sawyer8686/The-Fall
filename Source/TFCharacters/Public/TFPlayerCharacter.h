// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TFCharacterBase.h"
#include "TFKeyHolderInterface.h"
#include "TFInventoryHolderInterface.h"
#include "Blueprint/UserWidget.h"
#include "TFPlayerCharacter.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
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

DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventoryToggled, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnBackpackEquipRequested, int32, float);

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

#pragma region Input

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SneakAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LockAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InventoryAction;

#pragma endregion Input

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

#pragma region Input Handlers

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void SprintOn();
	void SprintOff();
	void SneakOn();
	void SneakOff();
	void PlayerJump();
	void InteractPressed();
	void LockPressed();
	void InventoryPressed();

#pragma endregion Input Handlers

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

#pragma region Accessors

	UTFStaminaComponent* GetStaminaComponent() const { return StaminaComponent; }
	UTFStatsComponent* GetStatsComponent() const { return StatsComponent; }
	UTFInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }
	UTFInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
	UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }
	bool IsSprinting() const { return bIsSprinting; }

#pragma endregion Accessors

#pragma region Key Collection

protected:

	UPROPERTY(VisibleAnywhere, Category = "Keys")
	TSet<FName> CollectedKeys;

public:

	virtual bool HasKey(FName KeyID) const override;
	virtual void AddKey(FName KeyID) override;
	virtual bool RemoveKey(FName KeyID) override;
	virtual void OnKeyAdded(FName KeyID) {}
	virtual void OnKeyRemoved(FName KeyID) {}

#pragma endregion Key Collection

#pragma region Inventory

public:

	FOnInventoryToggled OnInventoryToggled;
	FOnBackpackEquipRequested OnBackpackEquipRequested;

	virtual bool HasBackpack() const override;
	virtual bool ActivateBackpack(int32 Slots, float WeightLimit) override;
	virtual void SetPendingBackpackActor(AActor* Actor) override;
	virtual bool AddItem(const FItemData& Item) override;
	virtual bool RemoveItem(FName ItemID, int32 Quantity = 1) override;
	virtual bool HasItem(FName ItemID) const override;
	virtual bool HasSpaceForItem(const FItemData& Item) const override;
	virtual bool CanCarryWeight(float AdditionalWeight) const override;
	virtual int32 GetFreeSlots() const override;
	virtual float GetRemainingCapacity() const override;

	bool DropItem(FName ItemID);
	void ConfirmBackpackEquip();
	void CancelBackpackEquip();

private:

	bool bInventoryOpen = false;
	bool bConfirmDialogOpen = false;
	int32 PendingBackpackSlots = 0;
	float PendingBackpackWeightLimit = 0.0f;
	TWeakObjectPtr<AActor> PendingBackpackActor;

#pragma endregion Inventory

public:

	ATFPlayerCharacter();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};