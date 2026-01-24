// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InterfacesModule.h"
#include "Misc/ConfigCacheIni.h"

INTERFACES_API DECLARE_LOG_CATEGORY_EXTERN(LogTFInteraction, Log, All);
INTERFACES_API DECLARE_LOG_CATEGORY_EXTERN(LogTFDoor, Log, All);
INTERFACES_API DECLARE_LOG_CATEGORY_EXTERN(LogTFItem, Log, All);
INTERFACES_API DECLARE_LOG_CATEGORY_EXTERN(LogTFCharacter, Log, All);
INTERFACES_API DECLARE_LOG_CATEGORY_EXTERN(LogTFStats, Log, All);
INTERFACES_API DECLARE_LOG_CATEGORY_EXTERN(LogTFContainer, Log, All);

namespace TFStatNames
{
	const FName Hunger = FName(TEXT("Hunger"));
	const FName Thirst = FName(TEXT("Thirst"));
}


namespace TFConfigUtils
{
	
	inline bool LoadINISection(const FString& FileName, const FString& SectionName, FString& OutConfigFilePath, const FLogCategoryBase& LogCategory, bool bSilentOnMissing = false)
	{
		OutConfigFilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectConfigDir() / FileName);
		FConfigCacheIni::NormalizeConfigIniPath(OutConfigFilePath);

		if (!FPaths::FileExists(OutConfigFilePath))
		{
			if (!bSilentOnMissing)
			{
				UE_LOG_REF(LogCategory, Warning, TEXT("TFConfigUtils: %s not found at %s"), *FileName, *OutConfigFilePath);
			}
			return false;
		}

		FConfigFile ConfigFile;
		ConfigFile.Read(OutConfigFilePath);

		if (!ConfigFile.Contains(SectionName))
		{
			if (!bSilentOnMissing)
			{
				UE_LOG_REF(LogCategory, Warning, TEXT("TFConfigUtils: Section [%s] not found in %s"), *SectionName, *FileName);
			}
			return false;
		}

		return true;
	}

	template<typename TEnum>
	TEnum StringToEnum(const FString& StringValue, const TMap<FString, TEnum>& EnumMap, TEnum DefaultValue, bool* OutMatched = nullptr)
	{
		for (const auto& Pair : EnumMap)
		{
			if (StringValue.Equals(Pair.Key, ESearchCase::IgnoreCase))
			{
				if (OutMatched) *OutMatched = true;
				return Pair.Value;
			}
		}
		if (OutMatched) *OutMatched = false;
		return DefaultValue;
	}

	template<typename T>
	T* LoadAssetFromConfig(const FString& SectionName, const TCHAR* Key, const FString& ConfigFilePath, const FLogCategoryBase& LogCategory, const TCHAR* AssetTypeName)
	{
		FString StringValue;
		if (GConfig->GetString(*SectionName, Key, StringValue, ConfigFilePath) && !StringValue.IsEmpty())
		{
			if (T* LoadedAsset = LoadObject<T>(nullptr, *StringValue))
			{
				return LoadedAsset;
			}
			UE_LOG_REF(LogCategory, Warning, TEXT("TFConfigUtils: Failed to load %s: %s"), AssetTypeName, *StringValue);
		}
		return nullptr;
	}
}
