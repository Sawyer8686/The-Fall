// Copyright TF Project. All Rights Reserved.

#include "TFInventoryWidget.h"

#include "TFInventoryComponent.h"
#include "TFPlayerCharacter.h"
#include "TFItemActionHandler.h"

#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/Button.h"

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

	// Bind to inventory events
	CachedInventoryComponent->OnItemAdded.AddUObject(this, &UTFInventoryWidget::OnItemAdded);
	CachedInventoryComponent->OnItemRemoved.AddUObject(this, &UTFInventoryWidget::OnItemRemoved);
	CachedInventoryComponent->OnInventoryChanged.AddUObject(this, &UTFInventoryWidget::OnInventoryChanged);

	// Bind to toggle event
	Character->OnInventoryToggled.AddUObject(this, &UTFInventoryWidget::OnInventoryToggled);

	// Bind to key collection changes
	Character->OnKeyCollectionChanged.AddUObject(this, &UTFInventoryWidget::OnKeyCollectionChanged);
}

void UTFInventoryWidget::RebuildItemList()
{
	if (!ItemListContainer)
	{
		return;
	}

	ItemListContainer->ClearChildren();
	ActionHandlers.Empty();

	if (!CachedInventoryComponent)
	{
		return;
	}

	const TArray<FItemData>& Items = CachedInventoryComponent->GetItems();

	for (const FItemData& Item : Items)
	{
		// Riga
		UHorizontalBox* Row = NewObject<UHorizontalBox>(this);
		if (!Row)
		{
			continue;
		}

		// Importante: clipping per impedire draw oltre bounds
		Row->SetClipping(EWidgetClipping::ClipToBounds);

		// ------------------------------------
		// 1) TESTO: dentro una SizeBox con MaxDesiredWidth
		// ------------------------------------
		USizeBox* TextSizeBox = NewObject<USizeBox>(this);
		if (TextSizeBox)
		{
			TextSizeBox->SetMaxDesiredWidth(MaxItemTextWidth);
			TextSizeBox->SetClipping(EWidgetClipping::ClipToBounds);

			UTextBlock* ItemText = NewObject<UTextBlock>(this);
			if (ItemText)
			{
				const FString DisplayText = FString::Printf(TEXT("%s  (%.1f kg)"), *Item.ItemName.ToString(), Item.Weight);
				ItemText->SetText(FText::FromString(DisplayText));
				ItemText->SetColorAndOpacity(FSlateColor(FLinearColor::White));

				// niente wrap, una riga
				ItemText->SetAutoWrapText(false);

				// clipping sul testo
				ItemText->SetClipping(EWidgetClipping::ClipToBounds);

				TextSizeBox->AddChild(ItemText);
			}

			UHorizontalBoxSlot* TextSlot = Cast<UHorizontalBoxSlot>(Row->AddChild(TextSizeBox));
			if (TextSlot)
			{
				TextSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
				TextSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);

				// Non Fill totale: la SizeBox governa la larghezza del testo.
				FSlateChildSize AutoSize;
				AutoSize.SizeRule = ESlateSizeRule::Automatic;
				TextSlot->SetSize(AutoSize);

				TextSlot->SetPadding(FMargin(0.f, 0.f, TextToButtonsPadding, 0.f));
			}
		}

		// ------------------------------------
		// Action handler
		// ------------------------------------
		UTFItemActionHandler* Handler = NewObject<UTFItemActionHandler>(this);
		if (!Handler)
		{
			ItemListContainer->AddChild(Row);
			continue;
		}

		Handler->ItemID = Item.ItemID;
		Handler->OwnerWidget = this;
		ActionHandlers.Add(Handler);

		// ------------------------------------
		// 2) Pulsanti (Auto)
		// ------------------------------------
		UButton* ExamineButton = NewObject<UButton>(this);
		if (ExamineButton)
		{
			UTextBlock* ExamineLabel = NewObject<UTextBlock>(this);
			if (ExamineLabel)
			{
				ExamineLabel->SetText(FText::FromString(TEXT("Esamina")));
				ExamineLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White));
				ExamineButton->AddChild(ExamineLabel);
			}

			ExamineButton->OnClicked.AddDynamic(Handler, &UTFItemActionHandler::OnExamineClicked);

			if (UHorizontalBoxSlot* BtnSlot = Cast<UHorizontalBoxSlot>(Row->AddChild(ExamineButton)))
			{
				BtnSlot->SetPadding(FMargin(ButtonPadding, 0.f));
				BtnSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);

				FSlateChildSize AutoSize;
				AutoSize.SizeRule = ESlateSizeRule::Automatic;
				BtnSlot->SetSize(AutoSize);
			}
		}

		UButton* DiscardButton = NewObject<UButton>(this);
		if (DiscardButton)
		{
			UTextBlock* DiscardLabel = NewObject<UTextBlock>(this);
			if (DiscardLabel)
			{
				DiscardLabel->SetText(FText::FromString(TEXT("Scarta")));
				DiscardLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White));
				DiscardButton->AddChild(DiscardLabel);
			}

			DiscardButton->OnClicked.AddDynamic(Handler, &UTFItemActionHandler::OnDiscardClicked);

			if (UHorizontalBoxSlot* BtnSlot = Cast<UHorizontalBoxSlot>(Row->AddChild(DiscardButton)))
			{
				BtnSlot->SetPadding(FMargin(ButtonPadding, 0.f));
				BtnSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);

				FSlateChildSize AutoSize;
				AutoSize.SizeRule = ESlateSizeRule::Automatic;
				BtnSlot->SetSize(AutoSize);
			}
		}

		ItemListContainer->AddChild(Row);
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

	RebuildItemList();
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
