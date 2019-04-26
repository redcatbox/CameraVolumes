//Dmitriy Barannik aka redbox, 2019

/**
* Dynamic camera volume actor.
* Calculates volume extents every tick when activated.
*/

#pragma once

#include "CameraVolumeActor.h"
#include "CameraVolumeDynamicActor.generated.h"

UCLASS()
class CAMERAVOLUMES_API ACameraVolumeDynamicActor : public ACameraVolumeActor
{
	GENERATED_BODY()

public:
	ACameraVolumeDynamicActor();
};