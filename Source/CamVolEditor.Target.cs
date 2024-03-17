
using UnrealBuildTool;
using System.Collections.Generic;

public class CamVolEditorTarget : TargetRules
{
	public CamVolEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("CamVol");
	}
}
