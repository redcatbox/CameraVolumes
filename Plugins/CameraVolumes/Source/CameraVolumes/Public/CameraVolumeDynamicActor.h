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
	virtual void Tick(float DeltaTime) override;
	virtual void UpdateVolume() override;

	UFUNCTION()
		virtual void SetActive(bool bNewActive);

	/** Update volume extents for dynamic camera volume */
	UFUNCTION(BlueprintCallable, Category = "CameraVolumes")
		virtual void UpdateVolumeExtents();

protected:
	UPROPERTY()
		bool bActive;

	UPROPERTY()
		FVector OldVolumeLocation;

	UPROPERTY()
		FVector NewVolumeLocation;
};