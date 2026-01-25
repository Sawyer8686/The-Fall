// Copyright TF Project. All Rights Reserved.

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
				"Components",
				"WorldEnvironment",
				"Inventory",
				"Interfaces"
			}
			);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"TF"
            }
			);

		CircularlyReferencedDependentModules.Add("TF");

		// Add TF include path for circular dependency
		PrivateIncludePaths.Add(System.IO.Path.Combine(ModuleDirectory, "../TF"));

	}
}
