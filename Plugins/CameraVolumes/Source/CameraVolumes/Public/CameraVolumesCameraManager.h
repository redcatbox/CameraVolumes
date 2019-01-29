//Dmitriy Barannik aka redbox, 2019

/**
* Player camera manager performs camera calculations according to default camera parameters from player pawn or parameters from camera volumes.
*/

#pragma once

#include "Engine/Engine.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "CameraVolumeDynamicActor.h"
#include "CameraVolumesFunctionLibrary.h"
#include "CameraVolumesCharacter.h"
#include "CameraVolumesCameraManager.generated.h"

UCLASS()
class CAMERAVOLUMES_API ACameraVolumesCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	ACameraVolumesCameraManager();
	virtual void UpdateCamera(float DeltaTime) override;

	/** This condition used for optimization purpose. Player character will enable/disable it according to overlapping camera volumes. */
	UPROPERTY()
		bool bCheckCameraVolumes;

	/** Set transition according to side info */
	UFUNCTION()
		virtual void SetTransitionBySideInfo(ACameraVolumeActor* CameraVolume, ESide Side);

	/** Calculate new camera parameters */
	UFUNCTION()
		virtual void CalcNewCameraParams(ACameraVolumeActor* CameraVolume, float DeltaTime);

	/** Should perform camera calculations? Use this to enable/disable camera updates if it's necessary. */
	UFUNCTION(BlueprintCallable, Meta = (Category = "CameraVolumes"))
		virtual void SetUpdateCamera(bool bNewUpdateCamera);

protected:
	UPROPERTY()
		class ACameraVolumesCharacter* PlayerCharacter;

	UPROPERTY()
		FVector PlayerCharacterLocation;

	UPROPERTY()
		TArray<AActor*> OverlappingActors;

	UPROPERTY()
		class ACameraVolumeActor* CamVolCurrent;

	UPROPERTY()
		class ACameraVolumeActor* CamVolPrevious;

	UPROPERTY()
		FVector OldCameraLocation;

	UPROPERTY()
		FVector NewCameraLocation;

	UPROPERTY()
		FQuat OldCameraRotation;

	UPROPERTY()
		FQuat NewCameraRotation;

	UPROPERTY()
		float OldCameraFOV;

	UPROPERTY()
		float NewCameraFOV;

	UPROPERTY()
		bool bNeedsSmoothTransition;

	UPROPERTY()
		float SmoothTransitionAlpha;

	UPROPERTY()
		float SmoothTransitionTime;

	UPROPERTY()
		bool bNeedsCutTransition;

	UPROPERTY()
		bool bUpdateCamera;
};