// redbox, 2021

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

UCLASS(Config = CameraVolumes, AutoExpandCategories = (CameraSettings, "CameraSettings | DefaultParameters", "CameraSettings | AdditionalParameters", "CameraSettings | CameraCollision", "CameraSettings | CameraControlRotation"
#if DEAD_ZONES
	, "CameraSettings | DeadZone"
#endif
	))
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
	virtual void SetDefaultCameraLocation(FVector NewDefaultCameraLocation);

	// Default camera RELATIVE focal point
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CameraSettings | DefaultParameters")
	FVector DefaultCameraFocalPoint;

	// Set default camera focal point
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
	virtual void SetDefaultCameraFocalPoint(FVector NewDefaultCameraFocalPoint);

	// Default camera roll
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CameraSettings | DefaultParameters")
	float DefaultCameraRoll;

	// Set default camera roll
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
	virtual void SetDefaultCameraRoll(float NewDefaultCameraRoll);

	UPROPERTY()
	FQuat DefaultCameraRotation;

	// Default camera FOV. For perspective cameras.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters", Meta = (UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360", Units = deg))
	float DefaultCameraFieldOfView;

	// Default camera OrthoWidth. For orthographic cameras.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters")
	float DefaultCameraOrthoWidth;

	// Should camera use location lag?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters")
	bool bEnableCameraLocationLag;

	// Camera location lag speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters", Meta = (EditCondition = "bEnableCameraLocationLag", ClampMin = "0", ClampMax = "1000", UIMin = "0", UIMax = "1000"))
	float CameraLocationLagSpeed;

	// Should camera use rotation lag?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters")
	bool bEnableCameraRotationLag;

	// Camera rotation lag speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters", Meta = (EditCondition = "bEnableCameraRotationLag", ClampMin = "0", ClampMax = "1000", UIMin = "0", UIMax = "1000"))
	float CameraRotationLagSpeed;

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


#if 0 //DEAD_ZONES
	// Dead zone
public:
	// Should use screen-space dead zone to toggle camera movement?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DeadZone")
	bool bUseDeadZone;

	// Dead zone extent (in screen percentage)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DeadZone", Meta = (EditCondition = "bUseDeadZone"))
	FVector2D DeadZoneExtent;

	// Dead zone offset from the center of the screen (in screen percentage)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DeadZone", Meta = (EditCondition = "bUseDeadZone"))
	FVector2D DeadZoneOffset;

	// Should automatically calculate dead zone roll?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DeadZone", Meta = (EditCondition = "bUseDeadZone"))
	bool bShouldCalculateDeadZoneRoll;

#if WITH_EDITORONLY_DATA
public:
	// Should preview dead zone (editor only)?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DeadZone")
	bool bPreviewDeadZone;

protected:
	UPROPERTY(Config)
	FString DeadZonePreviewMaterialPath;

	UPROPERTY()
	UMaterialInterface* DeadZonePreviewMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* DeadZonePreviewMID;
#endif

#if WITH_EDITOR
public:
	void UpdateDeadZonePreview(const FDeadZoneTransform InDeadZoneTransform);
#endif
#endif


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
	UFUNCTION()
	virtual void UpdateCamera(FVector& CameraLocation, FQuat& CameraRotation, float CameraFOV_OW);

	// Update camera component parameters
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
	virtual void UpdateCameraComponent();

#if WITH_EDITOR
public:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
