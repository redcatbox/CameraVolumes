//redbox, 2019

/**
* Player camera manager performs camera calculations according to camera parameters from camera component or camera volume.
* Contains OnCameraVolumeChanged delegate.
*/

#pragma once

#include "Camera/PlayerCameraManager.h"
#include "CameraVolumesFunctionLibrary.h"
#include "CameraVolumesCameraManager.generated.h"

/** Delegate for notification when camera volume is changed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCameraVolumeChangedSignature, ACameraVolumeActor*, CameraVolume, FSideInfo, PassedSideInfo);

UCLASS(Config = CameraVolumes)
class CAMERAVOLUMES_API ACameraVolumesCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	ACameraVolumesCameraManager();
	virtual void UpdateCamera(float DeltaTime) override;

	/** Set transition according to side info */
	UFUNCTION()
		virtual void SetTransitionBySideInfo(ACameraVolumeActor* CameraVolume, FSideInfo SideInfo);

	/** Calculate new camera parameters */
	UFUNCTION()
		virtual void CalcNewCameraParams(ACameraVolumeActor* CameraVolume, float DeltaTime);

	/** Should perform camera updates? */
	UPROPERTY(BlueprintReadOnly, Category = CameraVolumes)
		bool bUpdateCamera;

	/** Set perform camera updates. */
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual void SetUpdateCamera(bool bNewUpdateCamera);

	/** Should check for camera volumes? */
	UPROPERTY(BlueprintReadOnly, Category = CameraVolumes)
		bool bCheckCameraVolumes;

	/** Set check for camera volumes. Used by Player Character according to overlapping camera volumes. */
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual void SetCheckCameraVolumes(bool bNewCheck);

	/** Should perform camera blocking calculations? */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = CameraVolumes)
		bool bPerformBlockingCalculations;

	/** Set perform camera blocking calculations */
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual void SetPerformBlockingCalculations(bool bNewPerformBlockingCalculations);

	/** Calculate screen world extent at depth */
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual FVector2D CalculateScreenWorldExtentAtDepth(float Depth);

	/** OnCameraVolumeChanged event signature */
	UPROPERTY(BlueprintAssignable, Category = CameraVolumes)
		FCameraVolumeChangedSignature OnCameraVolumeChanged;

	/** Get new calculated camera location. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = CameraVolumes)
		virtual FVector GetNewCameraLocation() { return NewCameraLocation; }

	/** Get new calculated camera rotation. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = CameraVolumes)
		virtual FRotator GetNewCameraRotation() { return NewCameraRotation.Rotator(); }

	/** Is provided world location in dead zone? */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = CameraVolumes)
		virtual bool IsInDeadZone(FVector WorldLocationToCheck);

protected:
	UPROPERTY()
		class APawn* PlayerPawn;

	UPROPERTY()
		bool bUsePlayerPawnControlRotation;
	
	UPROPERTY()
		FVector PlayerPawnLocation;

	UPROPERTY()
		FVector PlayerPawnLocationOld;

	UPROPERTY()
		class UCameraVolumesCameraComponent* CameraComponent;

	UPROPERTY(BlueprintReadOnly, Category = CameraVolumes)
		class ACameraVolumeActor* CameraVolumeCurrent;

	UPROPERTY()
		class ACameraVolumeActor* CameraVolumePrevious;

	UPROPERTY()
		FVector OldCameraLocation;

	UPROPERTY()
		FVector NewCameraLocation;

	UPROPERTY()
		FVector NewCameraLocationFinal;

	UPROPERTY()
		FVector NewCameraFocalPoint;

	UPROPERTY()
		FQuat OldCameraRotation;

	UPROPERTY()
		FQuat NewCameraRotation;

	UPROPERTY()
		FQuat NewCameraRotationFinal;

	UPROPERTY()
		float OldCameraFOV_OW;

	/** Used as FOV or OrthoWidth depending on bIsCameraOrthographic */
	UPROPERTY()
		float NewCameraFOV_OW;

	UPROPERTY()
		float NewCameraFOV_OWFinal;

	UPROPERTY()
		bool bIsCameraStatic;

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
		bool bBlockingCalculations;

	UPROPERTY()
		bool bBroadcastOnCameraVolumeChanged;

	UPROPERTY()
		ACameraVolumeActor* BroadcastCameraVolume;

	UPROPERTY()
		FSideInfo BroadcastSideInfo;
};
