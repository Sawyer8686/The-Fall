#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TFItemsData.generated.h"

USTRUCT(BlueprintType)
struct FTFItemsData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemMaxStackSize = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SingleItemWeight = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDurability = -1.0; // 0-1 as percentile, <0 not used

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* SkeletalMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseStaticMesh = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ItemName = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ItemDescription = FText::GetEmpty();

	

	FORCEINLINE bool IsValid() const
	{
		return ItemID.IsNone() && ItemMaxStackSize > 0;
	}

	FORCEINLINE void Clear()
	{
		ItemID = NAME_None;
		ItemMaxStackSize = 0;
		MaxDurability = 0;
		Icon = nullptr;
		Mesh = nullptr;
		SkeletalMesh = nullptr;
		ItemName = FText::GetEmpty();
		ItemDescription = FText::GetEmpty();
	}
};