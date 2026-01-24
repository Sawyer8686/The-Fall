// Copyright TF Project. All Rights Reserved.

#include "TFInventoryWidget.h"
#include "TFInventoryItemViewData.h"

#include "TFInventoryComponent.h"
#include "TFPlayerCharacter.h"

#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

#include "Kismet/GameplayStatics.h"

void UTFInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeInventoryComponent();

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
		if (ATFPlayerCharacter* Character = Cast<ATFPlayerCharacter>(PlayerPawn))
		{
			Character->OnInventoryToggled.RemoveAll(this);
			Character->OnKeyCollectionChanged.RemoveAll(this);
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

	CachedInventoryComponent->OnItemAdded.AddUObject(this, &UTFInventoryWidget::OnItemAdded);
	CachedInventoryComponent->OnItemRemoved.AddUObject(this, &UTFInventoryWidget::OnItemRemoved);
	CachedInventoryComponent->OnInventoryChanged.AddUObject(this, &UTFInventoryWidget::OnInventoryChanged);

	Character->OnInventoryToggled.AddUObject(this, &UTFInventoryWidget::OnInventoryToggled);
	Character->OnKeyCollectionChanged.AddUObject(this, &UTFInventoryWidget::OnKeyCollectionChanged);
}

void UTFInventoryWidget::PopulateListView()
{
	if (!ItemListView)
	{
		return;
	}

	ItemListView->ClearListItems();
	ListItems.Empty();

	if (!CachedInventoryComponent)
	{
		return;
	}

	const TArray<FItemData>& Items = CachedInventoryComponent->GetItems();

	for (const FItemData& Item : Items)
	{
		UTFInventoryItemViewData* ViewData = NewObject<UTFInventoryItemViewData>(this);
		ViewData->ItemData = Item;
		ViewData->OwnerWidget = this;

		ListItems.Add(ViewData);
		ItemListView->AddItem(ViewData);
	}
}

void UTFInventoryWidget::RebuildKeychainList()
{
	if (!KeychainContainer)
	{
		return;
	}

	KeychainContainer->ClearChildren();

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

	const TMap<FName, FText>& Keys = Character->GetCollectedKeys();

	if (Keys.Num() == 0)
	{
		KeychainContainer->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	KeychainContainer->SetVisibility(ESlateVisibility::Visible);

	for (const auto& KeyPair : Keys)
	{
		UTextBlock* KeyText = NewObject<UTextBlock>(this);
		if (KeyText)
		{
			KeyText->SetText(KeyPair.Value);
			KeyText->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.8f, 0.3f)));

			UVerticalBoxSlot* ASlot = Cast<UVerticalBoxSlot>(KeychainContainer->AddChild(KeyText));
			if (ASlot)
			{
				ASlot->SetPadding(FMargin(0.f, 2.f));
			}
		}
	}
}

void UTFInventoryWidget::OnKeyCollectionChanged()
{
	RebuildKeychainList();
}

void UTFInventoryWidget::UpdateWeightDisplay(float CurrentWeight, float MaxWeight)
{
	if (WeightText)
	{
		WeightText->SetText(
			FText::FromString(FString::Printf(TEXT("%.1f / %.1f kg"), CurrentWeight, MaxWeight))
		);
	}

	if (WeightBar && MaxWeight > 0.0f)
	{
		const float Percent = CurrentWeight / MaxWeight;
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

	const int32 UsedSlots = CachedInventoryComponent->GetUsedSlots();
	const int32 TotalSlots = CachedInventoryComponent->GetBackpackSlots();

	SlotsText->SetText(
		FText::FromString(FString::Printf(TEXT("%d / %d slots"), UsedSlots, TotalSlots))
	);
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
		const float Alpha = (WeightPercent - MediumWeightThreshold) / (HighWeightThreshold - MediumWeightThreshold);
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
	PopulateListView();
	UpdateSlotDisplay();
}

void UTFInventoryWidget::OnItemRemoved(FName ItemID, int32 Quantity)
{
	PopulateListView();
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
		CurrentExaminedItemID = NAME_None;

		if (DescriptionText)
		{
			DescriptionText->SetText(FText::GetEmpty());
		}

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

	PopulateListView();
	RebuildKeychainList();
	UpdateWeightDisplay(CachedInventoryComponent->GetCurrentWeight(), CachedInventoryComponent->GetBackpackWeightLimit());
	UpdateSlotDisplay();
}

void UTFInventoryWidget::ExamineItem(FName ItemID)
{
	if (!DescriptionText || !CachedInventoryComponent)
	{
		return;
	}

	if (CurrentExaminedItemID == ItemID)
	{
		DescriptionText->SetText(FText::GetEmpty());
		CurrentExaminedItemID = NAME_None;
		return;
	}

	if (const FItemData* Item = CachedInventoryComponent->GetItem(ItemID))
	{
		DescriptionText->SetText(Item->ItemDescription);
		CurrentExaminedItemID = ItemID;
	}
}

void UTFInventoryWidget::DiscardItem(FName ItemID)
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

	Character->DropItem(ItemID);

	if (CurrentExaminedItemID == ItemID)
	{
		CurrentExaminedItemID = NAME_None;

		if (DescriptionText)
		{
			DescriptionText->SetText(FText::GetEmpty());
		}
	}
}
