// redbox, 2025

#include "CameraVolumesModule.h"
#include "CameraVolumesCameraManager.h"
#if SHOWDEBUG
#include "Engine/Engine.h"
#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"
#include "Kismet/GameplayStatics.h"
#endif

#define LOCTEXT_NAMESPACE "FCameraVolumesModule"

void FCameraVolumesModule::StartupModule()
{
#if SHOWDEBUG
	if (!IsRunningDedicatedServer())
	{
		AHUD::OnShowDebugInfo.AddStatic(&FCameraVolumesModule::OnShowDebugInfo);
	}
#endif
}

void FCameraVolumesModule::ShutdownModule()
{
}

#if SHOWDEBUG
void FCameraVolumesModule::OnShowDebugInfo(class AHUD* HUD, class UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
	static const FName NAME_CameraVolumes("CameraVolumes");
	if (Canvas)
	{
		if (HUD->ShouldDisplayDebug(NAME_CameraVolumes))
		{
			FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
			DisplayDebugManager.SetFont(GEngine->GetSmallFont());
			DisplayDebugManager.SetDrawColor(FColor::Yellow);
			DisplayDebugManager.DrawString(TEXT("CAMERA VOLUMES"));

			APlayerCameraManager* PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(HUD, 0);
			ACameraVolumesCameraManager* CameraVolumesCameraManager = Cast<ACameraVolumesCameraManager>(PlayerCameraManager);
			if (IsValid(CameraVolumesCameraManager))
			{
				CameraVolumesCameraManager->ShowDebugInfo(Canvas);
			}
		}
	}
}
#endif

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCameraVolumesModule, CameraVolumes)
