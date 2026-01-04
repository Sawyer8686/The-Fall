// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TFEditorTarget : TargetRules
{
	public TFEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("TF");
		ExtraModuleNames.Add("TFCharacters");
		ExtraModuleNames.Add("TFCoreData");
		ExtraModuleNames.Add("TFItems");
		ExtraModuleNames.Add("TFInventory");
		
	}
}
