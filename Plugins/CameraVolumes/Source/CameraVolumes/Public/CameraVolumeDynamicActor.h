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
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
		virtual void SetActive(bool bNewActive);

protected:
	UPROPERTY()
		bool bActive;
};