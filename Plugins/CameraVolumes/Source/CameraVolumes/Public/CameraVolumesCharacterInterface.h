//Dmitriy Barannik aka redbox, 2019

// Base interface for characters using camera volumes
#pragma once

<<<<<<< HEAD
<<<<<<< HEAD
//#include "UObject/ObjectMacros.h"
//#include "UObject/Interface.h"
#include "CameraVolumesCameraComponent.h"
#include "CameraVolumesCharacterInterface.generated.h"

UINTERFACE(Meta = (CannotImplementInterfaceInBlueprint = true))
=======
#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"
#include "CameraVolumesCameraComponent.h"
#include "CameraVolumesCharacterInterface.generated.h"

UINTERFACE()
>>>>>>> ec14146... added character interface
=======
//#include "UObject/ObjectMacros.h"
//#include "UObject/Interface.h"
#include "CameraVolumesCameraComponent.h"
#include "CameraVolumesCharacterInterface.generated.h"

UINTERFACE(Meta = (CannotImplementInterfaceInBlueprint = true))
>>>>>>> 9adb816... finished with 2d support
class CAMERAVOLUMES_API UCameraVolumesCharacterInterface : public UInterface
{
	GENERATED_BODY()
};

class CAMERAVOLUMES_API ICameraVolumesCharacterInterface
{
	GENERATED_BODY()

protected:
<<<<<<< HEAD
<<<<<<< HEAD
	UFUNCTION()
		virtual void OnCapsuleComponentBeginOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) = 0;

	UFUNCTION()
		virtual void OnCapsuleComponentEndOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) = 0;

public:
	/** Returns CameraComponent subobject */
	UFUNCTION(BlueprintCallable, Category = "CameraVolumes")
		virtual class UCameraVolumesCameraComponent* GetCameraComponent() const = 0;
=======
	//UFUNCTION(BlueprintCallable, Category = "CameraVolumes")
		virtual void OnCapsuleComponentBeginOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
=======
	UFUNCTION()
		virtual void OnCapsuleComponentBeginOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) = 0;
>>>>>>> 9adb816... finished with 2d support

	UFUNCTION()
		virtual void OnCapsuleComponentEndOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) = 0;

public:
	/** Returns CameraComponent subobject */
<<<<<<< HEAD
	//UFUNCTION(BlueprintCallable, Category = "CameraVolumes")
		virtual UCameraVolumesCameraComponent* GetCameraComponent() const;
>>>>>>> ec14146... added character interface
=======
	UFUNCTION(BlueprintCallable, Category = "CameraVolumes")
		virtual class UCameraVolumesCameraComponent* GetCameraComponent() const = 0;
>>>>>>> 9adb816... finished with 2d support
};