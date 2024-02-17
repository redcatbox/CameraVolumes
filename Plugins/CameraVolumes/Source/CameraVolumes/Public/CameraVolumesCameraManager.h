// redbox, 2024

/**
 * Player camera manager performs camera calculations according to camera parameters from camera component or camera volume.
 * Contains OnCameraVolumeChanged delegate.
 */

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "CameraVolumesTypes.h"
#include "CameraVolumesCameraManager.generated.h"

// Delegate for notification when camera volume is changed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCameraVolumeChangedSignature, class ACameraVolumeActor*, CameraVolume, FSideInfo, PassedSideInfo);

UCLASS(Config = CameraVolumes)
class CAMERAVOLUMES_API ACameraVolumesCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	ACameraVolumesCameraManager(const FObjectInitializer& ObjectInitializer);
	virtual void UpdateCamera(float DeltaTime) override;

protected:
	// Calculate new camera parameters
	void CalculateCameraParams(float DeltaTime);

	// Set transition according to side info
	void SetTransitionBySideInfo(ACameraVolumeActor* CameraVolume, const FSideInfo& SideInfo);

	// Calculate camera transitions and interpolations
	void CalculateTransitions(float DeltaTime);

#if 0 //DEAD_ZONES
	// Process dead zone camera behavior
	void CalculateDeadZone();
#endif

public:
	// Should perform camera updates?
	UPROPERTY(BlueprintReadOnly, Category = CameraVolumes)
	bool bUpdateCamera;

	// Set perform camera updates.
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
	void SetUpdateCamera(bool bShouldUpdateCamera);

	// Should process camera volumes?
	UPROPERTY(BlueprintReadOnly, Category = CameraVolumes)
	bool bProcessCameraVolumes;

	// Set process camera volumes. Used by Player Character according to overlapping camera volumes.
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
	void SetProcessCameraVolumes(bool bShouldProcess);

	// Should perform camera blocking calculations?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CameraVolumes)
	bool bPerformBlockingCalculations;

	// Set perform camera blocking calculations
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
	virtual void SetPerformBlockingCalculations(bool bShouldPerformBlockingCalculations);

	// Calculate screen world extent at depth
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
	FVector2D CalculateScreenWorldExtentAtDepth(float Depth);

	// Get new calculated camera location.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = CameraVolumes)
	const FVector& GetCameraLocationNew() const { return CameraLocationNew; }

	// Get new calculated camera location.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = CameraVolumes)
	const FVector& GetCameraFocalPointNew() const { return CameraFocalPointNew; }

	// Get new calculated camera rotation.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = CameraVolumes)
	const FRotator GetCameraRotationNew() const { return CameraRotationNew.Rotator(); }

	// Reset first pass calculations
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
	void ResetFirstPass() { bFirstPass = true; }

	// OnCameraVolumeChanged event signature
	UPROPERTY(BlueprintAssignable, Category = CameraVolumes)
	FCameraVolumeChangedSignature OnCameraVolumeChanged;

protected:
	UPROPERTY()
	class APawn* PlayerPawn;

	UPROPERTY()
	bool bUsePlayerPawnControlRotation;

	UPROPERTY()
	FVector PlayerPawnLocationOld;

	UPROPERTY()
	FVector PlayerPawnLocation;

	UPROPERTY()
	class UCameraVolumesCameraComponent* CameraComponent;

	UPROPERTY(BlueprintReadOnly, Category = CameraVolumes)
	ACameraVolumeActor* CameraVolumeCurrent;

	UPROPERTY()
	ACameraVolumeActor* CameraVolumePrevious;

	UPROPERTY()
	FMinimalViewInfo CameraViewInfoFinal;

	UPROPERTY()
	FVector CameraLocationSourceOld;

	UPROPERTY()
	FVector CameraLocationSourceNew;

	UPROPERTY()
	FVector CameraLocationOld;

	UPROPERTY()
	FVector CameraLocationNew;

	UPROPERTY()
	FVector CameraLocationNewFixed;

	UPROPERTY()
	FVector CameraLocationFinalNew;

	UPROPERTY()
	float CameraOffset;

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

#if 0 //DEAD_ZONES
	UPROPERTY()
	bool bUseDeadZone;

	UPROPERTY()
	FVector2D DeadZoneExtent;

	UPROPERTY()
	FVector2D DeadZoneOffset;

	UPROPERTY()
	bool bShouldCalculateDeadZoneRoll;

	UPROPERTY()
	float DeadZoneRoll;

	UPROPERTY()
	bool bIsInDeadZone;

	UPROPERTY()
	FVector DeadZoneEdgePoint;
#endif

	UPROPERTY()
	bool bIsCameraStatic;

	UPROPERTY()
	TEnumAsByte<ECameraProjectionMode::Type> CameraProjectionModeNew;

	UPROPERTY()
	TEnumAsByte<ECameraProjectionMode::Type> CameraProjectionModeOld;

	UPROPERTY()
	bool bNeedsSmoothTransition;

	UPROPERTY()
	bool bSmoothTransitionInterrupted;

#if 0 //DEAD_ZONES
	UPROPERTY()
	bool bSmoothTransitionInDeadZone;
#endif

	UPROPERTY()
	bool bSmoothTransitionJustStarted;

	UPROPERTY()
	float SmoothTransitionSpeed;

	UPROPERTY()
	float SmoothTransitionAlpha;

	UPROPERTY()
	float SmoothTransitionAlphaEase;

	UPROPERTY()
	uint8 SmoothTransitionEasingFunc;

	UPROPERTY()
	float EasingFuncBlendExp;

	UPROPERTY()
	int32 EasingFuncSteps;

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
