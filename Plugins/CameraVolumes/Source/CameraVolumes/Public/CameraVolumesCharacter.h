//Dmitriy Barannik aka redbox, 2019

/**
* Player character stores default camera parameters and camera component.
* Player character uses BeginOverlap event of it's capsule collision primitive, to get overlapped camera volume and store it in special array of camera volume actors.
* Player character uses EndOverlap event of it's capsule collision primitive, to remove camera volume from array of camera volume actors.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CameraVolumeActor.h"
<<<<<<< HEAD
<<<<<<< HEAD
#include "CameraVolumesCharacterInterface.h"
#include "CameraVolumesCharacter.generated.h"

UCLASS()
class CAMERAVOLUMES_API ACameraVolumesCharacter : public ACharacter, public ICameraVolumesCharacterInterface
=======
#include "CameraVolumesCameraComponent.h"
#include "CameraVolumesCharacter.generated.h"

UCLASS()
class CAMERAVOLUMES_API ACameraVolumesCharacter : public ACharacter
>>>>>>> b8d6390... refactoring to match paper2d integration
=======
#include "CameraVolumesCharacterInterface.h"
#include "CameraVolumesCharacter.generated.h"

UCLASS()
class CAMERAVOLUMES_API ACameraVolumesCharacter : public ACharacter, public ICameraVolumesCharacterInterface
>>>>>>> ec14146... added character interface
{
	GENERATED_BODY()

	/** Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
		class UCameraVolumesCameraComponent* CameraComponent;

	virtual void OnCapsuleComponentBeginOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnCapsuleComponentEndOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

public:
	ACameraVolumesCharacter();
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
	virtual class UCameraVolumesCameraComponent* GetCameraComponent() const override;
=======

	/** Returns CameraComponent subobject */
	FORCEINLINE class UCameraVolumesCameraComponent* GetCameraComponent() const { return CameraComponent; }
>>>>>>> b8d6390... refactoring to match paper2d integration
=======
	virtual UCameraVolumesCameraComponent* GetCameraComponent() const override;
>>>>>>> ec14146... added character interface
=======
	virtual class UCameraVolumesCameraComponent* GetCameraComponent() const override;
>>>>>>> 9adb816... finished with 2d support
};
