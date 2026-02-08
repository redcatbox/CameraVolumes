// redbox, 2025

/**
 * Camera component that must be used with camera volumes.
 * Contains various camera parameters.
 * Contains overlapped camera volumes.
 * Updates final camera parameters provided by camera manager.
 * Can do camera collision test and camera relative rotation similar to SpringArmComponent.
 */

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "CameraVolumesTypes.h"
#include "CameraVolumesCameraComponent.generated.h"

UCLASS(Config = CameraVolumes, AutoExpandCategories = (CameraSettings, "CameraSettings | DefaultParameters", "CameraSettings | Lag", "CameraSettings | AdditionalParameters", "CameraSettings | CameraCollision", "CameraSettings | CameraControlRotation"))
class CAMERAVOLUMES_API UCameraVolumesCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	UCameraVolumesCameraComponent();


	// Default parameters
public:
	// Default camera RELATIVE location
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CameraSettings | DefaultParameters")
	FVector DefaultCameraLocation;

	// Set default camera location
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
	void SetDefaultCameraLocation(FVector NewDefaultCameraLocation);

	// Default camera RELATIVE focal point
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CameraSettings | DefaultParameters")
	FVector DefaultCameraFocalPoint;

	// Set default camera focal point
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
	void SetDefaultCameraFocalPoint(FVector NewDefaultCameraFocalPoint);

	// Default camera roll
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CameraSettings | DefaultParameters")
	float DefaultCameraRoll;

	// Set default camera roll
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
	void SetDefaultCameraRoll(float NewDefaultCameraRoll);

protected:
	UPROPERTY()
	FQuat DefaultCameraRotation;

public:
	const FQuat& GetDefaultCameraRotation() const { return DefaultCameraRotation; }

	// Default camera FOV. For perspective cameras.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters", Meta = (UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360", Units = deg))
	float DefaultCameraFieldOfView;

	// Default camera OrthoWidth. For orthographic cameras.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters")
	float DefaultCameraOrthoWidth;


	// Should camera use location lag?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | Lag")
	bool bEnableCameraLocationLag;

	// Camera location lag speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | Lag", Meta = (EditCondition = "bEnableCameraLocationLag", ClampMin = "0", ClampMax = "1000", UIMin = "0", UIMax = "1000"))
	float CameraLocationLagSpeed;

	// Should camera use rotation lag?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | Lag")
	bool bEnableCameraRotationLag;

	// Camera rotation lag speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | Lag", Meta = (EditCondition = "bEnableCameraRotationLag", ClampMin = "0", ClampMax = "1000", UIMin = "0", UIMax = "1000"))
	float CameraRotationLagSpeed;

	/**
	 * If bUseCameraLagSubstepping is true, sub-step camera damping so that it handles fluctuating frame rates well (though this comes at a cost).
	 * @see CameraLagMaxTimeStep
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings", AdvancedDisplay)
	bool bUseCameraLagSubstepping;

	/**
	 * If true and camera location lag is enabled, draws markers at the camera target (in green) and the lagged position (in yellow).
	 * A line is drawn between the two locations, in green normally but in red if the distance to the lag target has been clamped (by CameraLagMaxDistance).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | Lag")
	bool bDrawDebugLagMarkers;

	/** Max time step used when sub-stepping camera lag. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | Lag", AdvancedDisplay, meta = (editcondition = "bUseCameraLagSubstepping", ClampMin = "0.005", ClampMax = "0.5", UIMin = "0.005", UIMax = "0.5"))
	float CameraLagMaxTimeStep;

	/** Max distance the camera target may lag behind the current location. If set to zero, no max distance is enforced. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | Lag", meta = (editcondition = "bEnableCameraLag", ClampMin = "0.0", UIMin = "0.0"))
	float CameraLagMaxDistance;

	/** If true AND the view target is simulating using physics then use the same max timestep cap as the physics system. Prevents camera jitter when delta time is clamped within Chaos Physics. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | Lag")
	uint32 bClampToMaxPhysicsDeltaTime : 1;


	// Should camera use FOV interpolation? For perspective cameras.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters")
	bool bEnableCameraFOVInterp;

	// Camera FOV interpolation speed.  For perspective cameras.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters", Meta = (EditCondition = "bEnableCameraFOVInterp", ClampMin = "0", ClampMax = "1000", UIMin = "0", UIMax = "1000"))
	float CameraFOVInterpSpeed;

	// Should camera use OrthoWidth interpolation? For orthographic cameras.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters")
	bool bEnableCameraOrthoWidthInterp;

	// Camera OrthoWidth interpolation speed. For orthographic cameras.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters", Meta = (EditCondition = "bEnableCameraOrthoWidthInterp", ClampMin = "0", ClampMax = "1000", UIMin = "0", UIMax = "1000"))
	float CameraOrthoWidthInterpSpeed;


	// Additional parameters
public:
	// Should camera use additional (WORLD-SPACE) parameters?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | AdditionalParameters")
	bool bUseAdditionalCameraParams;

	// Additional location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | AdditionalParameters", Meta = (EditCondition = "bUseAdditionalCameraParams"))
	FVector AdditionalCameraLocation;

	// Additional rotation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | AdditionalParameters", Meta = (EditCondition = "bUseAdditionalCameraParams"))
	FRotator AdditionalCameraRotation;

	// Additional FOV. For perspective cameras.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | AdditionalParameters", Meta = (EditCondition = "bUseAdditionalCameraParams"))
	float AdditionalCameraFOV;

	// Additional OrthoWidth. For orthographic cameras.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | AdditionalParameters", Meta = (EditCondition = "bUseAdditionalCameraParams"))
	float AdditionalCameraOrthoWidth;


	// Camera collision
public:
	// Do a camera collision test using ProbeChannel and ProbeSize to prevent camera clipping into level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | CameraCollision")
	bool bDoCollisionTest;

	// How big should the query probe sphere be (in unreal units)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | CameraCollision", Meta = (EditCondition = "bDoCollisionTest"))
	float ProbeSize;

	// Collision channel of the query probe (defaults to ECC_Camera)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | CameraCollision", Meta = (EditCondition = "bDoCollisionTest"))
	TEnumAsByte<ECollisionChannel> ProbeChannel;


	// Camera rotation
public:
	// Should update control rotation from camera rotation when not in UsePawnControlRotation mode?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | CameraControlRotation")
	bool bUpdateControlRotationFromCameraRotation;

	// Should use pawn control rotation when it's possible?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | CameraControlRotation")
	bool bUsePawnControlRotationCV;

	// Should use pitch?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | CameraControlRotation", Meta = (EditCondition = "bUsePawnControlRotationCV"))
	bool bInheritPitchCV;

	// Should use yaw?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | CameraControlRotation", Meta = (EditCondition = "bUsePawnControlRotationCV"))
	bool bInheritYawCV;

	// Should use roll?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | CameraControlRotation", Meta = (EditCondition = "bUsePawnControlRotationCV"))
	bool bInheritRollCV;

	// Overlapping camera volumes
	UPROPERTY(BlueprintReadOnly, Category = CameraVolumes)
	TSet<class ACameraVolumeActor*> OverlappingCameraVolumes;

	// Should update camera parameters?
	UPROPERTY(BlueprintReadWrite, Category = CameraVolumes)
	bool bUpdateCamera;

	// Updates camera by camera manager
	void UpdateCamera(FMinimalViewInfo& InViewInfo);

#if WITH_EDITOR
public:
	void UpdateCameraComponent();

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
