//Dmitriy Barannik aka redbox, 2019

/**
* Player camera manager process camera calculations according to default camera parameters from player pawn or parameters from camera volumes.
* You can find information about basic algorithm in the end of file.
*/

#pragma once

#include "CoreMinimal.h"
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

	/** This condition can be used for optimization purpose. You can disable it on loading/cutscenes/etc. */
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
		ACameraVolumesCharacter* PlayerPawn;

	UPROPERTY()
		FVector PlayerPawnLocation;

	UPROPERTY()
		TArray<AActor*> OverlappingActors;

	UPROPERTY()
		ACameraVolumeActor* CamVolCurrent;

	UPROPERTY()
		ACameraVolumeActor* CamVolPrevious;

	UPROPERTY()
		FVector OldCameraLocation;

	UPROPERTY()
		FVector NewCameraLocation;

	UPROPERTY()
		FRotator OldCameraRotation;

	UPROPERTY()
		FRotator NewCameraRotation;

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