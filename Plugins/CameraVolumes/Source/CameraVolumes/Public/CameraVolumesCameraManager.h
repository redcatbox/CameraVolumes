// redbox, 2021

/**
 * Player camera manager performs camera calculations according to camera parameters from camera component or camera volume.
 * Contains OnCameraVolumeChanged delegate.
 */

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "CameraVolumesFunctionLibrary.h"
#include "CameraVolumesCameraManager.generated.h"

// Delegate for notification when camera volume is changed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCameraVolumeChangedSignature, ACameraVolumeActor*, CameraVolume, FSideInfo, PassedSideInfo);

UCLASS(Config = CameraVolumes)
class CAMERAVOLUMES_API ACameraVolumesCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	ACameraVolumesCameraManager(const FObjectInitializer& ObjectInitializer);
	virtual void UpdateCamera(float DeltaTime) override;

protected:
	// Set transition according to side info
	UFUNCTION()
		virtual void SetTransitionBySideInfo(ACameraVolumeActor* CameraVolume, FSideInfo SideInfo);

	// Calculate new camera parameters
	UFUNCTION()
		virtual void CalculateCameraParams(float DeltaTime);

	// Calculate camera transitions and interpolations
	UFUNCTION()
		virtual void CalculateTransitions(float DeltaTime);

	// Process dead zone camera behavior
	UFUNCTION()
		virtual void ProcessDeadZone();

public:
	// Should perform camera updates?
	UPROPERTY(BlueprintReadOnly, Category = CameraVolumes)
		bool bUpdateCamera;

	// OnCameraVolumeChanged event signature
	UPROPERTY(BlueprintAssignable, Category = CameraVolumes)
		FCameraVolumeChangedSignature OnCameraVolumeChanged;

	// Set perform camera updates.
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual void SetUpdateCamera(bool bNewUpdateCamera);

	// Should check for camera volumes?
	UPROPERTY(BlueprintReadOnly, Category = CameraVolumes)
		bool bCheckCameraVolumes;

	// Set check for camera volumes. Used by Player Character according to overlapping camera volumes.
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual void SetCheckCameraVolumes(bool bNewCheck);

	// Should perform camera blocking calculations?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CameraVolumes)
		bool bPerformBlockingCalculations;

	// Set perform camera blocking calculations
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual void SetPerformBlockingCalculations(bool bNewPerformBlockingCalculations);

	// Calculate screen world extent at depth
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual FVector2D CalculateScreenWorldExtentAtDepth(float Depth);

	// Get new calculated camera location.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = CameraVolumes)
		virtual FVector GetNewCameraLocation() { return CameraLocationNew; }

	// Get new calculated camera rotation.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = CameraVolumes)
		virtual FRotator GetNewCameraRotation() { return CameraRotationNew.Rotator(); }

	// Is location in dead zone?
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = CameraVolumes)
		virtual bool IsInDeadZone(FVector WorldLocationToCheck);

	// Reset first pass calculations
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual void ResetFirstPass() { bFirstPass = true; }
	
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
		FVector CameraLocationOld;

	UPROPERTY()
		FVector CameraLocationNew;

	UPROPERTY()
		FVector CameraLocationFinalNew;

	UPROPERTY()
		FVector CameraFocalPointNew;

	UPROPERTY()
		FQuat CameraRotationOld;

	UPROPERTY()
		FQuat CameraRotationNew;

	UPROPERTY()
		FQuat CameraRotationFinalNew;

	UPROPERTY()
		float CameraFOVOWOld;

	// Used as FOV or OrthoWidth depending on bIsCameraOrthographic
	UPROPERTY()
		float CameraFOVOWNew;

	UPROPERTY()
		float CameraFOVOWFinalNew;

	UPROPERTY()
		bool bFirstPass;

	UPROPERTY()
		bool bUseDeadZone;

	UPROPERTY()
		bool bIsInDeadZone;

	UPROPERTY()
		FVector2D DeadZoneExtent;

	UPROPERTY()
		FVector2D DeadZoneOffset;

	UPROPERTY()
		FVector DeadZoneWorldCenterOld;

	UPROPERTY()
		FVector DeadZoneWorldCenterNew;

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
