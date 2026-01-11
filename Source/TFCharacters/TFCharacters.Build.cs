// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TFCharacters : ModuleRules
{
	public TFCharacters (ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"EnhancedInput",
                "InputCore",
                "UMG",
				"Slate",
                "SlateCore"

            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine"	  
            }
			);
		
	}
}
