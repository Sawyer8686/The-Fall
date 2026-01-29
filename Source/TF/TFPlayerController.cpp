// Copyright TF Project. All Rights Reserved.

#include "TFPlayerController.h"
#include "TFPlayerCharacter.h"
#include "TFInteractionComponent.h"
#include "TFInventoryComponent.h"
#include "TFStaminaComponent.h"
#include "TFStatsComponent.h"
#include "TFBaseContainerActor.h"
#include "TFStatsWidget.h"
#include "TFStaminaWidget.h"
#include "TFDayNightWidget.h"
#include "TFInventoryWidget.h"
#include "TFBackpackIndicatorWidget.h"
#include "TFBackpackConfirmWidget.h"
#include "TFContainerWidget.h"
#include "TFCrosshairWidget.h"
#include "TFGameMode.h"
#include "TFDayNightCycle.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/UserWidget.h"

ATFPlayerController::ATFPlayerController()
{
}

void ATFPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Add input mapping context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext && !Subsystem->HasMappingContext(DefaultMappingContext))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Create HUD widgets
	CreateHUDWidgets();
}

void ATFPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyHUDWidgets();

	Super::EndPlay(EndPlayReason);
}

void ATFPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ATFPlayerCharacter* NewCharacter = Cast<ATFPlayerCharacter>(InPawn);
	CachedPlayerCharacter = NewCharacter;

	if (NewCharacter)
	{
		BindToCharacter(NewCharacter);
		InitializeWidgetBindings();
	}
}

void ATFPlayerController::OnUnPossess()
{
	if (CachedPlayerCharacter.IsValid())
	{
		UnbindFromCharacter(CachedPlayerCharacter.Get());
	}

	CachedPlayerCharacter = nullptr;

	Super::OnUnPossess();
}

void ATFPlayerController::BindToCharacter(ATFPlayerCharacter* InCharacter)
{
	if (!InCharacter)
	{
		return;
	}

	InCharacter->OnBackpackEquipRequested.AddUObject(this, &ATFPlayerController::HandleBackpackEquipRequested);
}

void ATFPlayerController::UnbindFromCharacter(ATFPlayerCharacter* InCharacter)
{
	if (!InCharacter)
	{
		return;
	}

	InCharacter->OnBackpackEquipRequested.RemoveAll(this);
}

void ATFPlayerController::HandleBackpackEquipRequested(int32 Slots, float WeightLimit)
{
	OpenBackpackConfirmDialog(Slots, WeightLimit);
}

#pragma region Widget Management

void ATFPlayerController::CreateHUDWidgets()
{
	// Stats Widget (hunger/thirst)
	if (StatsWidgetClass)
	{
		StatsWidget = CreateWidget<UTFStatsWidget>(this, StatsWidgetClass);
		if (StatsWidget)
		{
			StatsWidget->AddToViewport(0);
		}
	}

	// Stamina Widget
	if (StaminaWidgetClass)
	{
		StaminaWidget = CreateWidget<UTFStaminaWidget>(this, StaminaWidgetClass);
		if (StaminaWidget)
		{
			StaminaWidget->AddToViewport(0);
		}
	}

	// Day/Night Widget
	if (DayNightWidgetClass)
	{
		DayNightWidget = CreateWidget<UTFDayNightWidget>(this, DayNightWidgetClass);
		if (DayNightWidget)
		{
			DayNightWidget->AddToViewport(0);

			// Bind to DayNightCycle from GameMode
			if (ATFGameMode* GM = Cast<ATFGameMode>(GetWorld()->GetAuthGameMode()))
			{
				if (ATFDayNightCycle* DayNightCycle = GM->GetDayNightCycle())
				{
					DayNightWidget->SetDayNightCycle(DayNightCycle);
				}
			}
		}
	}

	// Inventory Widget (hidden by default)
	if (InventoryWidgetClass)
	{
		InventoryWidget = CreateWidget<UTFInventoryWidget>(this, InventoryWidgetClass);
		if (InventoryWidget)
		{
			InventoryWidget->AddToViewport(10);
			InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// Backpack Indicator Widget
	if (BackpackIndicatorWidgetClass)
	{
		BackpackIndicatorWidget = CreateWidget<UTFBackpackIndicatorWidget>(this, BackpackIndicatorWidgetClass);
		if (BackpackIndicatorWidget)
		{
			BackpackIndicatorWidget->AddToViewport(0);
		}
	}

	// Backpack Confirm Widget (hidden by default)
	if (BackpackConfirmWidgetClass)
	{
		BackpackConfirmWidget = CreateWidget<UTFBackpackConfirmWidget>(this, BackpackConfirmWidgetClass);
		if (BackpackConfirmWidget)
		{
			BackpackConfirmWidget->AddToViewport(20);
			BackpackConfirmWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// Container Widget (hidden by default)
	if (ContainerWidgetClass)
	{
		ContainerWidget = CreateWidget<UTFContainerWidget>(this, ContainerWidgetClass);
		if (ContainerWidget)
		{
			ContainerWidget->AddToViewport(10);
			ContainerWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// Crosshair Widget
	if (CrosshairWidgetClass)
	{
		CrosshairWidget = CreateWidget<UTFCrosshairWidget>(this, CrosshairWidgetClass);
		if (CrosshairWidget)
		{
			CrosshairWidget->AddToViewport(5);
		}
	}

	// Bind widgets to character if the pawn is already possessed (e.g. before BeginPlay).
	// If not yet possessed, InitializeWidgetBindings will no-op; OnPossess will call it again.
	InitializeWidgetBindings();
}

void ATFPlayerController::DestroyHUDWidgets()
{
	auto RemoveWidget = [](UUserWidget* Widget)
	{
		if (Widget)
		{
			Widget->RemoveFromParent();
		}
	};

	RemoveWidget(StatsWidget);
	RemoveWidget(StaminaWidget);
	RemoveWidget(DayNightWidget);
	RemoveWidget(InventoryWidget);
	RemoveWidget(BackpackIndicatorWidget);
	RemoveWidget(BackpackConfirmWidget);
	RemoveWidget(ContainerWidget);
	RemoveWidget(CrosshairWidget);

	StatsWidget = nullptr;
	StaminaWidget = nullptr;
	DayNightWidget = nullptr;
	InventoryWidget = nullptr;
	BackpackIndicatorWidget = nullptr;
	BackpackConfirmWidget = nullptr;
	ContainerWidget = nullptr;
	CrosshairWidget = nullptr;
}

void ATFPlayerController::InitializeWidgetBindings()
{
	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (!PlayerChar)
	{
		return;
	}

	// Bind Stats Widget to StatsComponent
	if (StatsWidget)
	{
		if (UTFStatsComponent* StatsComp = PlayerChar->GetStatsComponent())
		{
			StatsWidget->SetStatsComponent(StatsComp);
		}
	}

	// Bind Stamina Widget to StaminaComponent
	if (StaminaWidget)
	{
		if (UTFStaminaComponent* StaminaComp = PlayerChar->GetStaminaComponent())
		{
			StaminaWidget->SetStaminaComponent(StaminaComp);
		}
	}

	// Bind Inventory Widget to InventoryComponent
	if (InventoryWidget)
	{
		if (UTFInventoryComponent* InventoryComp = PlayerChar->GetInventoryComponent())
		{
			InventoryWidget->SetInventoryComponent(InventoryComp);
		}
	}

	// Bind Backpack Indicator to InventoryComponent
	if (BackpackIndicatorWidget)
	{
		if (UTFInventoryComponent* InventoryComp = PlayerChar->GetInventoryComponent())
		{
			BackpackIndicatorWidget->SetInventoryComponent(InventoryComp);
		}
	}
}

#pragma endregion Widget Management

void ATFPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATFPlayerController::HandleMove);
		}

		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATFPlayerController::HandleLook);
		}

		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ATFPlayerController::HandleJumpStarted);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ATFPlayerController::HandleJumpCompleted);
		}

		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ATFPlayerController::HandleSprintStarted);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATFPlayerController::HandleSprintCompleted);
		}

		if (SneakAction)
		{
			EnhancedInputComponent->BindAction(SneakAction, ETriggerEvent::Started, this, &ATFPlayerController::HandleSneakStarted);
			EnhancedInputComponent->BindAction(SneakAction, ETriggerEvent::Completed, this, &ATFPlayerController::HandleSneakCompleted);
		}

		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ATFPlayerController::HandleInteract);
		}

		if (InventoryAction)
		{
			EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &ATFPlayerController::HandleInventory);
		}

		if (DropBackpackAction)
		{
			EnhancedInputComponent->BindAction(DropBackpackAction, ETriggerEvent::Started, this, &ATFPlayerController::HandleDropBackpack);
		}
	}
}

ATFPlayerCharacter* ATFPlayerController::GetTFPlayerCharacter() const
{
	if (CachedPlayerCharacter.IsValid())
	{
		return CachedPlayerCharacter.Get();
	}
	return Cast<ATFPlayerCharacter>(GetPawn());
}

void ATFPlayerController::SetUIInputMode(bool bShowCursor)
{
	if (bShowCursor)
	{
		// Stop all movement immediately when opening UI
		if (ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter())
		{
			if (UCharacterMovementComponent* MovementComp = PlayerChar->GetCharacterMovement())
			{
				MovementComp->StopMovementImmediately();
			}
		}

		bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		SetIgnoreMoveInput(true);
		SetIgnoreLookInput(true);
	}
	else
	{
		bShowMouseCursor = false;
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		ResetIgnoreMoveInput();
		ResetIgnoreLookInput();
	}
}

void ATFPlayerController::ToggleInventory()
{
	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (!PlayerChar)
	{
		return;
	}

	UTFInventoryComponent* InventoryComp = PlayerChar->GetInventoryComponent();
	if (!InventoryComp || !InventoryComp->HasBackpack())
	{
		return;
	}

	if (bConfirmDialogOpen || bContainerOpen)
	{
		return;
	}

	bInventoryOpen = !bInventoryOpen;

	if (bInventoryOpen && PlayerChar->IsSprinting())
	{
		PlayerChar->StopSprinting();
	}

	// Show/hide inventory widget
	if (InventoryWidget)
	{
		InventoryWidget->SetVisibility(bInventoryOpen ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
		if (bInventoryOpen)
		{
			InventoryWidget->RefreshDisplay();
		}
	}

	SetUIInputMode(bInventoryOpen);
	OnInventoryToggled.Broadcast(bInventoryOpen);
}

void ATFPlayerController::OpenBackpackConfirmDialog(int32 Slots, float WeightLimit)
{
	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (!PlayerChar)
	{
		return;
	}

	bConfirmDialogOpen = true;

	if (PlayerChar->IsSprinting())
	{
		PlayerChar->StopSprinting();
	}

	// Show backpack confirm widget
	if (BackpackConfirmWidget)
	{
		BackpackConfirmWidget->SetBackpackInfo(Slots, WeightLimit);
		BackpackConfirmWidget->SetVisibility(ESlateVisibility::Visible);
	}

	SetUIInputMode(true);
	OnBackpackEquipRequested.Broadcast(Slots, WeightLimit);
}

void ATFPlayerController::CloseBackpackConfirmDialog(bool bConfirmed)
{
	bConfirmDialogOpen = false;

	// Hide backpack confirm widget
	if (BackpackConfirmWidget)
	{
		BackpackConfirmWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (PlayerChar)
	{
		if (bConfirmed)
		{
			PlayerChar->ConfirmBackpackEquip();
		}
		else
		{
			PlayerChar->CancelBackpackEquip();
		}
	}

	SetUIInputMode(false);
}

void ATFPlayerController::OpenContainer(ATFBaseContainerActor* Container)
{
	if (!Container || bContainerOpen || bInventoryOpen || bConfirmDialogOpen)
	{
		return;
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (!PlayerChar)
	{
		return;
	}

	CurrentContainer = Container;
	bContainerOpen = true;

	if (PlayerChar->IsSprinting())
	{
		PlayerChar->StopSprinting();
	}

	// Show container widget
	if (ContainerWidget)
	{
		ContainerWidget->SetContainerSource(Container);
		ContainerWidget->SetVisibility(ESlateVisibility::Visible);
	}

	SetUIInputMode(true);
}

void ATFPlayerController::CloseContainer()
{
	if (!bContainerOpen)
	{
		return;
	}

	bContainerOpen = false;
	CurrentContainer = nullptr;

	// Hide container widget
	if (ContainerWidget)
	{
		ContainerWidget->SetVisibility(ESlateVisibility::Hidden);
		ContainerWidget->SetContainerSource(nullptr);
	}

	SetUIInputMode(false);
}

#pragma region Input Handlers

void ATFPlayerController::HandleMove(const FInputActionValue& Value)
{
	if (IsUIBlockingInput())
	{
		return;
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (!PlayerChar)
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	PlayerChar->AddMovementInput(ForwardDirection, MovementVector.Y);
	PlayerChar->AddMovementInput(RightDirection, MovementVector.X);
}

void ATFPlayerController::HandleLook(const FInputActionValue& Value)
{
	if (IsUIBlockingInput())
	{
		return;
	}

	FVector2D LookAxisVector = Value.Get<FVector2D>();
	AddYawInput(LookAxisVector.X);
	AddPitchInput(-LookAxisVector.Y);
}

void ATFPlayerController::HandleJumpStarted()
{
	if (IsUIBlockingInput())
	{
		return;
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (PlayerChar)
	{
		PlayerChar->TryJump();
	}
}

void ATFPlayerController::HandleJumpCompleted()
{
	if (IsUIBlockingInput())
	{
		return;
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (PlayerChar)
	{
		PlayerChar->StopJumping();
	}
}

void ATFPlayerController::HandleSprintStarted()
{
	if (IsUIBlockingInput())
	{
		return;
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (PlayerChar)
	{
		PlayerChar->StartSprinting();
	}
}

void ATFPlayerController::HandleSprintCompleted()
{
	if (IsUIBlockingInput())
	{
		return;
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (PlayerChar)
	{
		PlayerChar->StopSprinting();
	}
}

void ATFPlayerController::HandleSneakStarted()
{
	if (IsUIBlockingInput())
	{
		return;
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (PlayerChar)
	{
		PlayerChar->StartSneaking();
	}
}

void ATFPlayerController::HandleSneakCompleted()
{
	if (IsUIBlockingInput())
	{
		return;
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (PlayerChar)
	{
		PlayerChar->StopSneaking();
	}
}

void ATFPlayerController::HandleInteract()
{
	if (IsUIBlockingInput())
	{
		return;
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (PlayerChar)
	{
		UTFInteractionComponent* InteractionComp = PlayerChar->GetInteractionComponent();
		if (InteractionComp)
		{
			InteractionComp->Interact();
		}
	}
}

void ATFPlayerController::HandleInventory()
{
	ToggleInventory();
}

void ATFPlayerController::HandleDropBackpack()
{
	if (bConfirmDialogOpen || bContainerOpen)
	{
		return;
	}

	ATFPlayerCharacter* PlayerChar = GetTFPlayerCharacter();
	if (PlayerChar)
	{
		// Close inventory if open before dropping
		if (bInventoryOpen)
		{
			ToggleInventory();
		}
		PlayerChar->DropBackpack();
	}
}

#pragma endregion Input Handlers
