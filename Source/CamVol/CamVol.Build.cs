
using UnrealBuildTool;

public class CamVol : ModuleRules
{
	public CamVol(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"EnhancedInput",
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
