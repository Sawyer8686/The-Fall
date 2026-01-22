// Copyright TF Project. All Rights Reserved.

#include "TFInventoryWidget.h"
#include "TFInventoryComponent.h"
#include "TFPlayerCharacter.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"

void UTFInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeInventoryComponent();

	// Start hidden
	SetVisibility(ESlateVisibility::Hidden);
}

void UTFInventoryWidget::NativeDestruct()
{
	if (CachedInventoryComponent)
	{
		CachedInventoryComponent->OnItemAdded.RemoveAll(this);
		CachedInventoryComponent->OnItemRemoved.RemoveAll(this);
		CachedInventoryComponent->OnInventoryChanged.RemoveAll(this);
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

void UTFInventoryWidget::InitializeInventoryComponent()
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

	// Bind to inventory events
	CachedInventoryComponent->OnItemAdded.AddUObject(this, &UTFInventoryWidget::OnItemAdded);
	CachedInventoryComponent->OnItemRemoved.AddUObject(this, &UTFInventoryWidget::OnItemRemoved);
	CachedInventoryComponent->OnInventoryChanged.AddUObject(this, &UTFInventoryWidget::OnInventoryChanged);

	// Bind to toggle event
	Character->OnInventoryToggled.AddUObject(this, &UTFInventoryWidget::OnInventoryToggled);
}

void UTFInventoryWidget::RebuildItemList()
{
	if (!ItemListContainer)
	{
		return;
	}

	ItemListContainer->ClearChildren();

	if (!CachedInventoryComponent)
	{
		return;
	}

	const TArray<FItemData>& Items = CachedInventoryComponent->GetItems();
	for (const FItemData& Item : Items)
	{
		UTextBlock* ItemEntry = NewObject<UTextBlock>(this);
		if (ItemEntry)
		{
			FString ItemText = FString::Printf(TEXT("%s  (%.1f kg)"), *Item.ItemName.ToString(), Item.Weight);
			ItemEntry->SetText(FText::FromString(ItemText));
			ItemEntry->SetColorAndOpacity(FSlateColor(FLinearColor::White));
			ItemListContainer->AddChild(ItemEntry);
		}
	}
}

void UTFInventoryWidget::UpdateWeightDisplay(float CurrentWeight, float MaxWeight)
{
	if (WeightText)
	{
		FText WeightDisplayText = FText::FromString(FString::Printf(TEXT("%.1f / %.1f kg"), CurrentWeight, MaxWeight));
		WeightText->SetText(WeightDisplayText);
	}

	if (WeightBar && MaxWeight > 0.0f)
	{
		float Percent = CurrentWeight / MaxWeight;
		WeightBar->SetPercent(Percent);
		UpdateWeightColor(Percent);
	}
}

void UTFInventoryWidget::UpdateSlotDisplay()
{
	if (!SlotsText || !CachedInventoryComponent)
	{
		return;
	}

	int32 UsedSlots = CachedInventoryComponent->GetUsedSlots();
	int32 TotalSlots = CachedInventoryComponent->GetBackpackSlots();

	FText SlotsDisplayText = FText::FromString(FString::Printf(TEXT("%d / %d slots"), UsedSlots, TotalSlots));
	SlotsText->SetText(SlotsDisplayText);
}

void UTFInventoryWidget::UpdateWeightColor(float WeightPercent)
{
	if (!WeightBar)
	{
		return;
	}

	FLinearColor TargetColor;

	if (WeightPercent < MediumWeightThreshold)
	{
		TargetColor = LowWeightColor;
	}
	else if (WeightPercent < HighWeightThreshold)
	{
		float Alpha = (WeightPercent - MediumWeightThreshold) / (HighWeightThreshold - MediumWeightThreshold);
		TargetColor = FMath::Lerp(MediumWeightColor, HighWeightColor, Alpha);
	}
	else
	{
		TargetColor = HighWeightColor;
	}

	WeightBar->SetFillColorAndOpacity(TargetColor);
}

void UTFInventoryWidget::OnItemAdded(const FItemData& Item)
{
	RebuildItemList();
	UpdateSlotDisplay();
}

void UTFInventoryWidget::OnItemRemoved(FName ItemID, int32 Quantity)
{
	RebuildItemList();
	UpdateSlotDisplay();
}

void UTFInventoryWidget::OnInventoryChanged(float CurrentWeight, float MaxWeight)
{
	UpdateWeightDisplay(CurrentWeight, MaxWeight);
}

void UTFInventoryWidget::OnInventoryToggled(bool bIsOpen)
{
	SetVisibility(bIsOpen ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

	if (bIsOpen)
	{
		RefreshDisplay();
	}
}

void UTFInventoryWidget::SetInventoryComponent(UTFInventoryComponent* NewInventoryComponent)
{
	if (CachedInventoryComponent)
	{
		CachedInventoryComponent->OnItemAdded.RemoveAll(this);
		CachedInventoryComponent->OnItemRemoved.RemoveAll(this);
		CachedInventoryComponent->OnInventoryChanged.RemoveAll(this);
	}

	CachedInventoryComponent = NewInventoryComponent;

	if (CachedInventoryComponent)
	{
		CachedInventoryComponent->OnItemAdded.AddUObject(this, &UTFInventoryWidget::OnItemAdded);
		CachedInventoryComponent->OnItemRemoved.AddUObject(this, &UTFInventoryWidget::OnItemRemoved);
		CachedInventoryComponent->OnInventoryChanged.AddUObject(this, &UTFInventoryWidget::OnInventoryChanged);
	}

	RefreshDisplay();
}

void UTFInventoryWidget::RefreshDisplay()
{
	if (!CachedInventoryComponent)
	{
		return;
	}

	RebuildItemList();
	UpdateWeightDisplay(CachedInventoryComponent->GetCurrentWeight(), CachedInventoryComponent->GetBackpackWeightLimit());
	UpdateSlotDisplay();
}
