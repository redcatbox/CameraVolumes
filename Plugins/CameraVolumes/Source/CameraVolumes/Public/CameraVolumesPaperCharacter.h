//Dmitriy Barannik aka redbox, 2019

/**
* Player character stores default camera parameters and camera component.
* Player character uses BeginOverlap event of it's capsule collision primitive, to get overlapped camera volume and store it in special array of camera volume actors.
* Player character uses EndOverlap event of it's capsule collision primitive, to remove camera volume from array of camera volume actors.
*/

#pragma once

#include "PaperCharacter.h"
#include "CameraVolumesCharacterInterface.h"
#include "CameraVolumesPaperCharacter.generated.h"

UCLASS()
class CAMERAVOLUMES_API ACameraVolumesPaperCharacter : public APaperCharacter, public ICameraVolumesCharacterInterface
{
	GENERATED_BODY()

	/** Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CameraVolumes, Meta = (AllowPrivateAccess = "true"))
		class UCameraVolumesCameraComponent* CameraComponent;

	virtual void OnCapsuleComponentBeginOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnCapsuleComponentEndOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

public:
	ACameraVolumesPaperCharacter();
	virtual void PostInitializeComponents() override;
	virtual class UCameraVolumesCameraComponent* GetCameraComponent() const override;
};
