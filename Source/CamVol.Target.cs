
using UnrealBuildTool;
using System.Collections.Generic;

public class CamVolTarget : TargetRules
{
	public CamVolTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.Add("CamVol");
	}
}
