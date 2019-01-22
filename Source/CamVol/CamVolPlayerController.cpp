
#include "CamVolPlayerController.h"
#include "CameraVolumesCameraManager.h"

ACamVolPlayerController::ACamVolPlayerController()
{
	PlayerCameraManagerClass = ACameraVolumesCameraManager::StaticClass();
}