// Copyright TF Project. All Rights Reserved.

#include "TFBackpackConfirmWidget.h"
#include "TFPlayerCharacter.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UTFBackpackConfirmWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeCharacter();

	SetVisibility(ESlateVisibility::Hidden);

	if (YesButton)
	{
		YesButton->OnClicked.AddDynamic(this, &UTFBackpackConfirmWidget::OnYesClicked);
	}

	if (NoButton)
	{
		NoButton->OnClicked.AddDynamic(this, &UTFBackpackConfirmWidget::OnNoClicked);
	}
}

void UTFBackpackConfirmWidget::NativeDestruct()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerPawn)
	{
		if (ATFPlayerCharacter* Character = Cast<ATFPlayerCharacter>(PlayerPawn))
		{
			Character->OnBackpackEquipRequested.RemoveAll(this);
		}
	}

	Super::NativeDestruct();
}

void UTFBackpackConfirmWidget::InitializeCharacter()
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

	Character->OnBackpackEquipRequested.AddUObject(this, &UTFBackpackConfirmWidget::OnBackpackEquipRequested);
}

void UTFBackpackConfirmWidget::OnBackpackEquipRequested(int32 Slots, float WeightLimit)
{
	SetVisibility(ESlateVisibility::Visible);

	if (QuestionText)
	{
		QuestionText->SetText(FText::FromString(TEXT("Vuoi equipaggiare lo zaino?")));
	}
}

void UTFBackpackConfirmWidget::OnYesClicked()
{
	SetVisibility(ESlateVisibility::Hidden);

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

	Character->ConfirmBackpackEquip();
}

void UTFBackpackConfirmWidget::OnNoClicked()
{
	SetVisibility(ESlateVisibility::Hidden);

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

	Character->CancelBackpackEquip();
}
