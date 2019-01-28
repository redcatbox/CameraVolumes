//Dmitriy Barannik aka redbox, 2019

/**
* Player camera manager performs camera calculations according to default camera parameters from player pawn or parameters from camera volumes.
*/

#pragma once

#include "Engine/Engine.h"
#include "Camera/PlayerCameraManager.h"
#include "CameraVolumeDynamicActor.h"
#include "CameraVolumesFunctionLibrary.h"
<<<<<<< HEAD
<<<<<<< HEAD
#include "CameraVolumesCameraManager.generated.h"

<<<<<<< HEAD
UCLASS(Config = CameraVolumes)
=======
UCLASS(Config=CameraVolumes)
>>>>>>> 429da0c... added Config/DefaultCameraVolumes.ini
=======
#include "CameraVolumesCharacter.h"
#include "CameraVolumesPaperCharacter.h"
=======
>>>>>>> ec14146... added character interface
#include "CameraVolumesCameraManager.generated.h"

<<<<<<< HEAD
UCLASS()
>>>>>>> b8d6390... refactoring to match paper2d integration
=======
UCLASS(Config = CameraVolumes)
>>>>>>> a8e85d6... fff
class CAMERAVOLUMES_API ACameraVolumesCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	ACameraVolumesCameraManager();
	virtual void UpdateCamera(float DeltaTime) override;

<<<<<<< HEAD
<<<<<<< HEAD
=======
	UPROPERTY(Config)
		bool bOnly2DCalculations;

	/** This condition used for optimization purpose. Player character will enable/disable it according to overlapping camera volumes. */
	UPROPERTY()
		bool bCheckCameraVolumes;

>>>>>>> 429da0c... added Config/DefaultCameraVolumes.ini
=======
>>>>>>> b8d6390... refactoring to match paper2d integration
	/** Set transition according to side info */
	UFUNCTION()
		virtual void SetTransitionBySideInfo(ACameraVolumeActor* CameraVolume, ESide Side);

	/** Calculate new camera parameters */
	UFUNCTION()
		virtual void CalcNewCameraParams(ACameraVolumeActor* CameraVolume, float DeltaTime);

	/** Should perform camera calculations? Use this to enable/disable camera updates if it's necessary. */
	UFUNCTION(BlueprintCallable, Meta = (Category = "CameraVolumes"))
		virtual void SetUpdateCamera(bool bNewUpdateCamera);

	/** Should check for camera volumes. Used by Player Character according to overlapping camera volumes. */
	UFUNCTION(BlueprintCallable, Meta = (Category = "CameraVolumes"))
		virtual void SetCheckCameraVolumes(bool bNewCheck);
	
protected:
<<<<<<< HEAD
<<<<<<< HEAD
	//UPROPERTY(Config)
	//	bool bTestConfig;
=======
	UPROPERTY(Config)
		bool bTestConfig;

>>>>>>> a8e85d6... fff
	UPROPERTY()
<<<<<<< HEAD
		class UCameraVolumesCameraComponent* CameraComponent;
=======
		class ACameraVolumesCharacter* PlayerCharacter;
>>>>>>> bc8bf29... in progress 2

	UPROPERTY()
<<<<<<< HEAD
		FVector PlayerPawnLocation;
=======
		class ACameraVolumesPaperCharacter* PlayerPaperCharacter;

=======
	//UPROPERTY(Config)
	//	bool bTestConfig;
>>>>>>> ec14146... added character interface
	UPROPERTY()
		class UCameraVolumesCameraComponent* CameraComponent;

	UPROPERTY()
<<<<<<< HEAD
		FVector PlayerLocation;
>>>>>>> b8d6390... refactoring to match paper2d integration
=======
		FVector PlayerPawnLocation;
>>>>>>> a8e85d6... fff

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
		bool bCheckCameraVolumes;

	UPROPERTY()
		bool bUpdateCamera;
};