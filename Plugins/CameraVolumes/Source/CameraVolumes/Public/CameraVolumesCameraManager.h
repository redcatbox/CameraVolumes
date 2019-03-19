//Dmitriy Barannik aka redbox, 2019

/**
* Player camera manager performs camera calculations according to default camera parameters from player pawn or parameters from camera volumes.
*/

#pragma once

#include "Engine/Engine.h"
#include "Camera/PlayerCameraManager.h"
#include "CameraVolumeDynamicActor.h"
#include "CameraVolumesFunctionLibrary.h"
#include "CameraVolumesCameraManager.generated.h"

UCLASS()
class CAMERAVOLUMES_API ACameraVolumesCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	ACameraVolumesCameraManager();
	virtual void UpdateCamera(float DeltaTime) override;

	/** Set transition according to side info */
	UFUNCTION()
		virtual void SetTransitionBySideInfo(ACameraVolumeActor* CameraVolume, ESide Side);

	/** Calculate new camera parameters */
	UFUNCTION()
		virtual void CalcNewCameraParams(ACameraVolumeActor* CameraVolume, float DeltaTime);

	/** Should perform camera calculations? Use this to enable/disable camera updates if it's necessary. */
	UFUNCTION(BlueprintCallable, Meta = (Category = CameraVolumes))
		virtual void SetUpdateCamera(bool bNewUpdateCamera);

	/** Should check for camera volumes. Used by Player Character according to overlapping camera volumes. */
	UFUNCTION(BlueprintCallable, Meta = (Category = CameraVolumes))
		virtual void SetCheckCameraVolumes(bool bNewCheck);
	
	/** Should perform camera blocking calculations? */
	UFUNCTION(BlueprintCallable, Meta = (Category = CameraVolumes))
		virtual void SetPerformBlockingCalculations(bool bNewPerformBlockingCalculations);

protected:
	UPROPERTY()
		class UCameraVolumesCameraComponent* CameraComponent;

	UPROPERTY()
		FVector PlayerPawnLocation;

	UPROPERTY()
		TArray<AActor*> OverlappingActors;

	UPROPERTY(BlueprintReadOnly, Category = CameraVolumes)
		class ACameraVolumeActor* CameraVolumeCurrent;

	UPROPERTY()
		class ACameraVolumeActor* CameraVolumePrevious;

	UPROPERTY()
		FVector OldCameraLocation;

	UPROPERTY()
		FVector NewCameraLocation;

	UPROPERTY()
		FQuat OldCameraRotation;

	UPROPERTY()
		FQuat NewCameraRotation;

	UPROPERTY()
		float OldCameraFOV_OW;

	/** Used as FOV or OrthoWidth depending on bIsCameraOrthographic */
	UPROPERTY()
		float NewCameraFOV_OW;

	UPROPERTY()
		bool bIsCameraOrthographic;

	UPROPERTY()
		bool bNeedsSmoothTransition;

	UPROPERTY()
		float SmoothTransitionSpeed;

	UPROPERTY()
		float SmoothTransitionAlpha;

	UPROPERTY()
		bool bNeedsCutTransition;

	UPROPERTY()
		bool bUpdateCamera;

	UPROPERTY()
		bool bCheckCameraVolumes;

	UPROPERTY()
		bool bPerformBlockingCalculations;
};