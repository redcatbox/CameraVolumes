//redbox, 2021

// Dynamic camera volume actor

#pragma once

#include "CoreMinimal.h"
#include "CameraVolumeActor.h"
#include "CameraVolumeDynamicActor.generated.h"

UCLASS()
class CAMERAVOLUMES_API ACameraVolumeDynamicActor : public ACameraVolumeActor
{
	GENERATED_BODY()

public:
	ACameraVolumeDynamicActor();
};
