#pragma once

#include "CoreMinimal.h"
#include "ECreatureType.generated.h"

UENUM(BlueprintType)
enum class ECreatureType : uint8
{
	CT_DEFAULT UMETA(DisplayName = "Default"),
	CT_HUMAN UMETA(DisplayName = "Human"),
	CT_SMALLANIMAL UMETA(DisplayName = "Small Herbivor"),
	CT_SMALLCARNIVOR UMETA(DisplayName = "Small Carnivor"),
	CT_MEDIUMANIMAL UMETA(DisplayName = "Medium Herbivor"),
	CT_MEDIUMCARNIVOR UMETA(DisplayName = "Medium Carnivor"),
	CT_LARGEANIMAL UMETA(DisplayName = "Large Herbivor"),
	CT_LARGECARNIVOR UMETA(DisplayName = "Large Carnivor"),
	CT_MONSTER UMETA(DisplayName = "Monsterous"),

};