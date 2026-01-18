// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class WorldEnvironment : ModuleRules
{
    public WorldEnvironment(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
        });
    }
}
