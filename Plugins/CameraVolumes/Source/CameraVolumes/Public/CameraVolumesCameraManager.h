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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCameraVolumeChangedSignature, const class ACameraVolumeActor*, CameraVolume, FSideInfo, PassedSideInfo);

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
	void SetTransitionBySideInfo(const ACameraVolumeActor* CameraVolume, const FSideInfo& SideInfo);

	// Calculate camera transitions and interpolations
	void CalculateTransitions(float DeltaTime);

public:
	// Should perform camera updates?
	UPROPERTY(BlueprintReadOnly, Category = CameraVolumes)
	bool bUpdateCamera;

	// Set perform camera updates.
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
	void SetUpdateCamera(bool bShouldUpdateCamera) { bUpdateCamera = bShouldUpdateCamera; }

	// Should process camera volumes?
	UPROPERTY(BlueprintReadOnly, Category = CameraVolumes)
	bool bProcessCameraVolumes;

	// Set process camera volumes. Used by Player Character according to overlapping camera volumes.
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
	void SetProcessCameraVolumes(bool bShouldProcess) { bProcessCameraVolumes = bShouldProcess; }

	// Should perform camera blocking calculations?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CameraVolumes)
	bool bPerformBlockingCalculations;

	// Set perform camera blocking calculations
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
	virtual void SetPerformBlockingCalculations(bool bShouldPerformBlockingCalculations) { bPerformBlockingCalculations = bShouldPerformBlockingCalculations; }

	// Calculate screen world extent at depth
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
	FVector2D CalculateScreenWorldExtentAtDepth(float Depth);

	// Get new calculated camera location.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = CameraVolumes)
	const FVector& GetCameraLocationNew() const { return CameraLocationNew; }

	// Get new calculated camera focal point.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = CameraVolumes)
	const FVector& GetCameraFocalPointNew() const { return CameraFocalPointNew; }

	// Get new calculated camera rotation.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = CameraVolumes)
	const FRotator GetCameraRotationNew() const { return CameraQuatNew.Rotator(); }

	// Get new calculated camera FOV.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = CameraVolumes)
	const float GetCameraFOVNew() const { return CameraFOVNew; }

	// Get new calculated camera OrthoWidth.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = CameraVolumes)
	const float GetCameraOrthoWidthNew() const { return CameraOrthoWidthNew; }

	// Get new calculated camera location (including additional camera params).
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = CameraVolumes)
	const FVector& GetCameraLocationFinal() const { return CameraLocationFinal; }

	// Get new calculated camera rotation (including additional camera params).
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = CameraVolumes)
	const FRotator GetCameraRotationFinal() const { return CameraQuatFinal.Rotator(); }

	// Get new calculated camera FOV (including additional camera params).
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = CameraVolumes)
	const float GetCameraFOVFinal() const { return CameraFOVFinal; }

	// Get new calculated camera OrthoWidth (including additional camera params).
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = CameraVolumes)
	const float GetCameraOrthoWidthFinal() const { return CameraOrthoWidthFinal; }

	// Reset first pass calculations
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
	void ResetFirstPass() { bFirstPass = true; }

	// OnCameraVolumeChanged event signature
	UPROPERTY(BlueprintAssignable, Category = CameraVolumes)
	FCameraVolumeChangedSignature OnCameraVolumeChanged;

protected:
	UPROPERTY()
	class APawn* PlayerPawn;

	FVector PlayerPawnLocation;
	bool bUsePlayerPawnControlRotation;

	UPROPERTY()
	class UCameraVolumesCameraComponent* CameraComponent;

	UPROPERTY(BlueprintReadOnly, Category = CameraVolumes)
	const ACameraVolumeActor* CameraVolumeCurrent;

	UPROPERTY()
	const ACameraVolumeActor* CameraVolumePrevious;

	UPROPERTY()
	FMinimalViewInfo CameraViewInfoFinal;

	FVector CameraLocationSourceOld;
	FVector CameraLocationSourceNew;
	FVector CameraLocationOld;
	FVector CameraLocationNew;
	FVector CameraLocationNewFixed;
	FVector CameraLocationFinal;

	FVector CameraFocalPointNew;
	FQuat CameraQuatOld;
	FQuat CameraQuatNew;
	FQuat CameraQuatFinal;

	float CameraFOVOld;
	float CameraFOVNew;
	float CameraFOVFinal;

	float CameraOrthoWidthOld;
	float CameraOrthoWidthNew;
	float CameraOrthoWidthFinal;

	bool bFirstPass;

	bool bIsCameraStatic;

	TEnumAsByte<ECameraProjectionMode::Type> CameraProjectionModeOld;
	TEnumAsByte<ECameraProjectionMode::Type> CameraProjectionModeNew;

	bool bNeedsSmoothTransition;
	bool bSmoothTransitionInterrupted;

	bool bSmoothTransitionJustStarted;
	float SmoothTransitionSpeed;
	float SmoothTransitionAlpha;
	float SmoothTransitionAlphaEase;

	uint8 SmoothTransitionEasingFunc;
	float EasingFuncBlendExp;
	int32 EasingFuncSteps;

	bool bNeedsCutTransition;

	bool bBlockingCalculations;

	bool bBroadcastOnCameraVolumeChanged;

	UPROPERTY()
	const ACameraVolumeActor* BroadcastCameraVolume;

	UPROPERTY()
	FSideInfo BroadcastSideInfo;
};
