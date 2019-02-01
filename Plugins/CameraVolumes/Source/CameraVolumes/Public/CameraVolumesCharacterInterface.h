//Dmitriy Barannik aka redbox, 2019

// Base interface for characters using camera volumes
#pragma once

#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"
#include "CameraVolumesCameraComponent.h"
#include "CameraVolumesCharacterInterface.generated.h"

UINTERFACE()
class CAMERAVOLUMES_API UCameraVolumesCharacterInterface : public UInterface
{
	GENERATED_BODY()
};

class CAMERAVOLUMES_API ICameraVolumesCharacterInterface
{
	GENERATED_BODY()

protected:
	//UFUNCTION(BlueprintCallable, Category = "CameraVolumes")
		virtual void OnCapsuleComponentBeginOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//UFUNCTION(BlueprintCallable, Category = "CameraVolumes")
		virtual void OnCapsuleComponentEndOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	/** Returns CameraComponent subobject */
	//UFUNCTION(BlueprintCallable, Category = "CameraVolumes")
		virtual UCameraVolumesCameraComponent* GetCameraComponent() const;
};