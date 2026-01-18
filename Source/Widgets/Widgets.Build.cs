// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Widgets : ModuleRules
{
	public Widgets (ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"UMG",
				"Slate",
				"SlateCore",
				"TFCharacters",
				"Components"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
                

            }
			);
		
	}
}
