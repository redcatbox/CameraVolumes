
#include "CamVolPlayerController.h"
#include "CameraVolumesCameraManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CamVolPlayerController)

ACamVolPlayerController::ACamVolPlayerController()
{
	PlayerCameraManagerClass = ACameraVolumesCameraManager::StaticClass();
}
