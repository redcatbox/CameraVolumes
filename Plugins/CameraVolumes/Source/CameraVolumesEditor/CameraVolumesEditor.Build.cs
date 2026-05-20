// redbox, 2026

using UnrealBuildTool;

public class CameraVolumesEditor : ModuleRules
{
	public CameraVolumesEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
			}
			);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
			);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"CameraVolumes"
			}
			);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
