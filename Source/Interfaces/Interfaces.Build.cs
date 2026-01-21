// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Interfaces : ModuleRules
{
	public Interfaces (ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// Define INTERFACES_EXPORTS when building this module
		PrivateDefinitions.Add("INTERFACES_EXPORTS=1");

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core"
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
