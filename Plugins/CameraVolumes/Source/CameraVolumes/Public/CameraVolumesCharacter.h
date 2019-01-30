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
#include "CameraVolumesCameraComponent.h"
#include "CameraVolumesCharacter.generated.h"

UCLASS()
class CAMERAVOLUMES_API ACameraVolumesCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
		class UCameraVolumesCameraComponent* CameraComponent;

	UFUNCTION()
		void OnCapsuleComponentBeginOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnCapsuleComponentEndOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	ACameraVolumesCharacter();

	/** Overlapping camera volumes */
	UPROPERTY()
		TArray<ACameraVolumeActor*> OverlappingCameraVolumes;

	/** Returns CameraComponent subobject */
	FORCEINLINE class UCameraVolumesCameraComponent* GetCameraComponent() const { return CameraComponent; }
};
