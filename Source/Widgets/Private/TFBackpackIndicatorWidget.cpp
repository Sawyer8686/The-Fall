// Copyright TF Project. All Rights Reserved.

#include "TFBackpackIndicatorWidget.h"
#include "TFInventoryComponent.h"
#include "TFPlayerCharacter.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UTFBackpackIndicatorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeInventoryComponent();
	UpdateVisibility();
}

void UTFBackpackIndicatorWidget::NativeDestruct()
{
	if (CachedInventoryComponent)
	{
		CachedInventoryComponent->OnBackpackActivated.RemoveAll(this);
		CachedInventoryComponent->OnBackpackDeactivated.RemoveAll(this);
		CachedInventoryComponent = nullptr;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerPawn)
	{
		ATFPlayerCharacter* Character = Cast<ATFPlayerCharacter>(PlayerPawn);
		if (Character)
		{
			Character->OnInventoryToggled.RemoveAll(this);
		}
	}

	Super::NativeDestruct();
}

void UTFBackpackIndicatorWidget::InitializeInventoryComponent()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		return;
	}

	ATFPlayerCharacter* Character = Cast<ATFPlayerCharacter>(PlayerPawn);
	if (!Character)
	{
		return;
	}

	CachedInventoryComponent = Character->GetInventoryComponent();
	if (!CachedInventoryComponent)
	{
		return;
	}

	CachedInventoryComponent->OnBackpackActivated.AddUObject(this, &UTFBackpackIndicatorWidget::OnBackpackActivated);
	CachedInventoryComponent->OnBackpackDeactivated.AddUObject(this, &UTFBackpackIndicatorWidget::OnBackpackDeactivated);

	Character->OnInventoryToggled.AddUObject(this, &UTFBackpackIndicatorWidget::OnInventoryToggled);
}

void UTFBackpackIndicatorWidget::UpdateVisibility()
{
	bool bHasBackpack = CachedInventoryComponent && CachedInventoryComponent->HasBackpack();

	SetVisibility(bHasBackpack ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

	if (StatusText && bHasBackpack)
	{
		StatusText->SetText(InventoryClosedText);
	}

	if (DropHintText)
	{
		DropHintText->SetText(bHasBackpack ? DropBackpackText : FText::GetEmpty());
	}
}

void UTFBackpackIndicatorWidget::OnBackpackActivated()
{
	UpdateVisibility();
}

void UTFBackpackIndicatorWidget::OnBackpackDeactivated()
{
	UpdateVisibility();
}

void UTFBackpackIndicatorWidget::OnInventoryToggled(bool bIsOpen)
{
	if (StatusText)
	{
		StatusText->SetText(bIsOpen ? InventoryOpenText : InventoryClosedText);
	}
}

void UTFBackpackIndicatorWidget::SetInventoryComponent(UTFInventoryComponent* NewInventoryComponent)
{
	if (CachedInventoryComponent)
	{
		CachedInventoryComponent->OnBackpackActivated.RemoveAll(this);
		CachedInventoryComponent->OnBackpackDeactivated.RemoveAll(this);
	}

	CachedInventoryComponent = NewInventoryComponent;

	if (CachedInventoryComponent)
	{
		CachedInventoryComponent->OnBackpackActivated.AddUObject(this, &UTFBackpackIndicatorWidget::OnBackpackActivated);
		CachedInventoryComponent->OnBackpackDeactivated.AddUObject(this, &UTFBackpackIndicatorWidget::OnBackpackDeactivated);
	}

	UpdateVisibility();
}
