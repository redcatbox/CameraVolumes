// redbox, 2024

/**
 * Interface for characters using camera volumes.
 * Returns camera component.
 * Returns collision primitive component and it's BeginOverlap and EndOverlap events.
 */

#pragma once

#include "CoreMinimal.h"
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

public:
	// Returns CameraComponent subobject
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
	virtual class UCameraVolumesCameraComponent* GetCameraComponent() const = 0;

	// Returns collision primitive subobject (CapsuleComponent for Characters)
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
	virtual class UPrimitiveComponent* GetCollisionPrimitiveComponent() const = 0;

protected:
	UFUNCTION()
	virtual void OnCollisionPrimitiveComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) = 0;

	UFUNCTION()
	virtual void OnCollisionPrimitiveComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) = 0;
};
