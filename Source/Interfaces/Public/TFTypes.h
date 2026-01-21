// Copyright TF Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InterfacesModule.h"
#include "Misc/ConfigCacheIni.h"

// Log Categories
INTERFACES_API DECLARE_LOG_CATEGORY_EXTERN(LogTFInteraction, Log, All);
INTERFACES_API DECLARE_LOG_CATEGORY_EXTERN(LogTFDoor, Log, All);
INTERFACES_API DECLARE_LOG_CATEGORY_EXTERN(LogTFItem, Log, All);
INTERFACES_API DECLARE_LOG_CATEGORY_EXTERN(LogTFCharacter, Log, All);
INTERFACES_API DECLARE_LOG_CATEGORY_EXTERN(LogTFStats, Log, All);

// Stat Names
namespace TFStatNames
{
	const FName Hunger = FName(TEXT("Hunger"));
	const FName Thirst = FName(TEXT("Thirst"));
}

/**
 * Utility functions for INI configuration loading
 */
namespace TFConfigUtils
{
	/**
	 * Loads an INI section from a config file.
	 * @param FileName - INI file name (e.g., "DoorConfig.ini")
	 * @param SectionName - Section name to load (e.g., "Door_MainEntrance")
	 * @param OutConfigFilePath - Outputs the normalized config file path for use with GConfig
	 * @param LogCategory - Log category for error messages
	 * @param bSilentOnMissing - If true, won't log warning when file/section is missing
	 * @return true if section was found and can be loaded
	 */
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

	/**
	 * Converts a string to enum value using a mapping.
	 * @param StringValue - The string to convert
	 * @param EnumMap - Map of string names to enum values
	 * @param DefaultValue - Value to return if string doesn't match
	 * @param OutMatched - Optional output: true if a match was found
	 * @return The matched enum value or DefaultValue if not found
	 */
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

	/**
	 * Loads an asset from INI config with error logging.
	 * @param SectionName - INI section name
	 * @param Key - INI key name
	 * @param ConfigFilePath - Path to the INI file
	 * @param LogCategory - Log category for error messages
	 * @param AssetTypeName - Human-readable name of asset type for logging
	 * @return Loaded asset or nullptr
	 */
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
