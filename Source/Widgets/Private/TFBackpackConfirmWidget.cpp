// Copyright TF Project. All Rights Reserved.

#include "TFBackpackConfirmWidget.h"
#include "TFPlayerController.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UTFBackpackConfirmWidget::NativeConstruct()
{
	Super::NativeConstruct();

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
	Super::NativeDestruct();
}

void UTFBackpackConfirmWidget::InitializeCharacter()
{
	// No longer needed - widget is managed by PlayerController
}

void UTFBackpackConfirmWidget::OnBackpackEquipRequested(int32 Slots, float WeightLimit)
{
	SetBackpackInfo(Slots, WeightLimit);
}

void UTFBackpackConfirmWidget::SetBackpackInfo(int32 Slots, float WeightLimit)
{
	if (QuestionText)
	{
		QuestionText->SetText(FText::FromString(
			FString::Printf(TEXT("Vuoi equipaggiare lo zaino?\n(%d slot, %.1f kg)"), Slots, WeightLimit)
		));
	}
}

void UTFBackpackConfirmWidget::OnYesClicked()
{
	if (ATFPlayerController* PC = Cast<ATFPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{
		PC->CloseBackpackConfirmDialog(true);
	}
}

void UTFBackpackConfirmWidget::OnNoClicked()
{
	if (ATFPlayerController* PC = Cast<ATFPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{
		PC->CloseBackpackConfirmDialog(false);
	}
}
