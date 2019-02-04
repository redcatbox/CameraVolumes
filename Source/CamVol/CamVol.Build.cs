// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CamVol : ModuleRules
{
	public CamVol(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"CameraVolumes",
            "Paper2D"
			}
		);
	}
}
