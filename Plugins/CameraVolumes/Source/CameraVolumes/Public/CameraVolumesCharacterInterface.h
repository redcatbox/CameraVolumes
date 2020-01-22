//Dmitriy Barannik aka redbox, 2019

// Base interface for characters using camera volumes
#pragma once

#include "CameraVolumesCameraComponent.h"
#include "Components/PrimitiveComponent.h"
#include "CameraVolumesCharacterInterface.generated.h"

UINTERFACE(Meta = (CannotImplementInterfaceInBlueprint = true))
class CAMERAVOLUMES_API UCameraVolumesCharacterInterface : public UInterface
{
	GENERATED_BODY()
};

class CAMERAVOLUMES_API ICameraVolumesCharacterInterface
{
	GENERATED_BODY()

protected:
	UFUNCTION()
		virtual void OnCapsuleComponentBeginOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) = 0;

	UFUNCTION()
		virtual void OnCapsuleComponentEndOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) = 0;

public:
	/** Returns CameraComponent subobject */
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual class UCameraVolumesCameraComponent* GetCameraComponent() const = 0;

	/** Returns collision primitive subobject (CapsuleComponent for Characters) */
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual class UPrimitiveComponent* GetCollisionPrimitiveComponent() const = 0;
};