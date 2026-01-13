// Fill out your copyright notice in the Description page of Project Settings.

#include "TFInteractionWidget.h"
#include "Components/TFInteractionComponent.h"
#include "TFPlayerCharacter.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Kismet/GameplayStatics.h"

void UTFInteractionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize interaction component
	InitializeInteractionComponent();

	// Hide by default
	SetVisibility(ESlateVisibility::Hidden);
	bIsShowing = false;
	CurrentAlpha = 0.0f;
	TargetAlpha = 0.0f;

	// Hide progress bar initially
	if (HoldProgressBar)
	{
		HoldProgressBar->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Set key hint if available
	if (KeyHintText)
	{
		KeyHintText->SetText(InputKeyHint);
	}
}

void UTFInteractionWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CachedInteractionComponent)
	{
		return;
	}

	// Update fade animation
	UpdateFade(InDeltaTime);

	// Update hold progress if holding
	if (CachedInteractionComponent->IsHolding())
	{
		UpdateHoldProgress(CachedInteractionComponent->GetHoldProgress());
	}
}

void UTFInteractionWidget::InitializeInteractionComponent()
{
	// Get player character
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		return;
	}

	// Try to cast to TFPlayerCharacter
	ATFPlayerCharacter* PlayerCharacter = Cast<ATFPlayerCharacter>(PlayerPawn);
	if (!PlayerCharacter)
	{
		return;
	}

	// Get interaction component
	CachedInteractionComponent = PlayerCharacter->GetInteractionComponent();
	if (!CachedInteractionComponent)
	{
		return;
	}

	// Bind to events
	CachedInteractionComponent->OnInteractionChanged.AddDynamic(this, &UTFInteractionWidget::OnInteractionChanged);
	CachedInteractionComponent->OnInteractionLost.AddDynamic(this, &UTFInteractionWidget::OnInteractionLost);
}

void UTFInteractionWidget::UpdateInteractionDisplay(const FInteractionData& Data)
{
	// Update main text
	if (InteractionText)
	{
		// Format: "Press [E] to [Action]"
		FText FormattedText = FText::Format(
			FText::FromString("Press {0} to {1}"),
			InputKeyHint,
			Data.InteractionText
		);
		InteractionText->SetText(FormattedText);
	}

	// Update secondary text
	if (SecondaryText)
	{
		if (!Data.SecondaryText.IsEmpty())
		{
			SecondaryText->SetText(Data.SecondaryText);
			SecondaryText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			SecondaryText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// Update icon
	if (InteractionIcon && Data.InteractionIcon)
	{
		InteractionIcon->SetBrushFromTexture(Data.InteractionIcon);
		InteractionIcon->SetVisibility(ESlateVisibility::Visible);
	}
	else if (InteractionIcon)
	{
		InteractionIcon->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Show/hide hold progress bar
	if (HoldProgressBar && bShowHoldProgress)
	{
		if (Data.InteractionDuration > 0.0f)
		{
			HoldProgressBar->SetVisibility(ESlateVisibility::Visible);
			HoldProgressBar->SetPercent(0.0f);
		}
		else
		{
			HoldProgressBar->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UTFInteractionWidget::UpdateHoldProgress(float Progress)
{
	if (HoldProgressBar && bShowHoldProgress)
	{
		HoldProgressBar->SetPercent(Progress);
	}
}

void UTFInteractionWidget::ShowWidget()
{
	if (bIsShowing)
	{
		return;
	}

	bIsShowing = true;
	TargetAlpha = 1.0f;
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UTFInteractionWidget::HideWidget()
{
	if (!bIsShowing)
	{
		return;
	}

	bIsShowing = false;
	TargetAlpha = 0.0f;
}

void UTFInteractionWidget::UpdateFade(float DeltaTime)
{
	if (FMath::IsNearlyEqual(CurrentAlpha, TargetAlpha, 0.01f))
	{
		CurrentAlpha = TargetAlpha;

		// Hide completely when faded out
		if (CurrentAlpha <= 0.0f)
		{
			SetVisibility(ESlateVisibility::Hidden);
		}

		return;
	}

	// Interpolate alpha
	float FadeSpeed = 1.0f / FadeDuration;
	CurrentAlpha = FMath::FInterpTo(CurrentAlpha, TargetAlpha, DeltaTime, FadeSpeed);

	// Apply alpha to widget
	SetRenderOpacity(CurrentAlpha);
}

void UTFInteractionWidget::OnInteractionChanged(AActor* InteractableActor, FInteractionData InteractionData)
{
	// Update display
	UpdateInteractionDisplay(InteractionData);

	// Show widget
	ShowWidget();
}

void UTFInteractionWidget::OnInteractionLost()
{
	// Hide widget
	HideWidget();

	// Reset progress bar
	if (HoldProgressBar)
	{
		HoldProgressBar->SetPercent(0.0f);
	}
}

void UTFInteractionWidget::SetInteractionComponent(UTFInteractionComponent* NewInteractionComponent)
{
	// Unbind from old component
	if (CachedInteractionComponent)
	{
		CachedInteractionComponent->OnInteractionChanged.RemoveDynamic(this, &UTFInteractionWidget::OnInteractionChanged);
		CachedInteractionComponent->OnInteractionLost.RemoveDynamic(this, &UTFInteractionWidget::OnInteractionLost);
	}

	// Set new component
	CachedInteractionComponent = NewInteractionComponent;

	// Bind to new component
	if (CachedInteractionComponent)
	{
		CachedInteractionComponent->OnInteractionChanged.AddDynamic(this, &UTFInteractionWidget::OnInteractionChanged);
		CachedInteractionComponent->OnInteractionLost.AddDynamic(this, &UTFInteractionWidget::OnInteractionLost);
	}
}