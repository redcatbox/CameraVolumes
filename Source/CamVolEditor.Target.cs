
using UnrealBuildTool;
using System.Collections.Generic;

public class CamVolEditorTarget : TargetRules
{
	public CamVolEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.Add("CamVol");
	}
}
