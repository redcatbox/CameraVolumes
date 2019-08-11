// Dmitriy Barannik aka redbox, 2019

/**
* Camera component contains overlapped camera volumes and updates final camera parameters provided by camera manager.
* Can do camera collision test similar to SpringArm.
*/

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "CameraVolumeActor.h"
#include "CameraVolumesCameraComponent.generated.h"

UCLASS(AutoExpandCategories = (CameraSettings, "CameraSettings | DefaultParameters", "CameraSettings | AdditionalParameters", "CameraSettings | CameraCollision", "CameraSettings | CameraRotation"))
class CAMERAVOLUMES_API UCameraVolumesCameraComponent : public UCameraComponent
{
	GENERATED_BODY()
	
public:
	UCameraVolumesCameraComponent();

	/** Default camera RELATIVE location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters")
		FVector DefaultCameraLocation;

	/** Set default camera location */
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual void SetDefaultCameraLocation(FVector NewDefaultCameraLocation);

	/** Default camera RELATIVE focal point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters")
		FVector DefaultCameraFocalPoint;

	/** Set default camera focal point */
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual void SetDefaultCameraFocalPoint(FVector NewDefaultCameraFocalPoint);

	/** Default camera roll */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters")
		float DefaultCameraRoll;

	/** Set default camera roll */
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual void SetDefaultCameraRoll(float NewDefaultCameraRoll);

	UPROPERTY()
		FQuat DefaultCameraRotation;

	/** Default camera FOV. For perspective cameras. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters", Meta = (UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0", Units = deg))
		float DefaultCameraFieldOfView;

	/** Default camera OrthoWidth. For orthographic cameras. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters")
		float DefaultCameraOrthoWidth;

	/** Should camera use location lag? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters")
		bool bEnableCameraLocationLag;

	/** Camera location lag speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters", Meta = (EditCondition = "bEnableCameraLocationLag", ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
		float CameraLocationLagSpeed;

	/** Should camera use rotation lag? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters")
		bool bEnableCameraRotationLag;

	/** Camera rotation lag speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters", Meta = (EditCondition = "bEnableCameraRotationLag", ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
		float CameraRotationLagSpeed;

	/** Should camera use FOV interpolation? For perspective cameras. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters")
		bool bEnableCameraFOVInterp;

	/** Camera FOV interpolation speed.  For perspective cameras. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters", Meta = (EditCondition = "bEnableCameraFOVInterp", ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
		float CameraFOVInterpSpeed;

	/** Should camera use OrthoWidth interpolation? For orthographic cameras. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters")
		bool bEnableCameraOrthoWidthInterp;

	/** Camera OrthoWidth interpolation speed. For orthographic cameras. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters", Meta = (EditCondition = "bEnableCameraOrthoWidthInterp", ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
		float CameraOrthoWidthInterpSpeed;

	/** Should camera use additional (WORLD-SPACE) parameters? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | AdditionalParameters")
		bool bUseAdditionalCameraParams;

	/** Additional location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | AdditionalParameters")
		FVector AdditionalCameraLocation;

	/** Additional rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | AdditionalParameters")
		FRotator AdditionalCameraRotation;

	/** Additional FOV. For perspective cameras. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | AdditionalParameters")
		float AdditionalCameraFOV;

	/** Additional OrthoWidth. For orthographic cameras. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | AdditionalParameters")
		float AdditionalCameraOrthoWidth;

	/** If true, do a collision test using ProbeChannel and ProbeSize to prevent camera clipping into level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | CameraCollision")
		bool bDoCollisionTest;

	/** How big should the query probe sphere be (in unreal units) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | CameraCollision", Meta = (EditCondition = "bDoCollisionTest"))
		float ProbeSize;

	/** Collision channel of the query probe (defaults to ECC_Camera) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | CameraCollision", Meta = (EditCondition = "bDoCollisionTest"))
		TEnumAsByte<ECollisionChannel> ProbeChannel;

	/** Should use pawn control rotation when it's possible? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | CameraRotation")
		bool bUsePawnControlRotationCV;

	/** Should use pitch? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | CameraRotation")
		bool bInheritPitchCV;

	/** Should use yaw? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | CameraRotation")
		bool bInheritYawCV;

	/** Should use roll? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | CameraRotation")
		bool bInheritRollCV;

	/** Overlapping camera volumes */
	UPROPERTY(BlueprintReadOnly, Category = CameraVolumes)
		TArray<ACameraVolumeActor*> OverlappingCameraVolumes;

	/** Updates camera by camera manager */
	UFUNCTION()
		virtual void UpdateCamera(FVector& CameraLocation, FVector& CameraFocalPoint, FQuat& CameraRotation, float CameraFOV, bool bIsCameraStatic);

	/** Get is camera uses orthographic projection mode */
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual bool GetIsCameraOrthographic() { return bIsCameraOrthographic; }

	/** Update camera component parameters */
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual void UpdateCameraComponent();

	/** Should update camera parameters? */
	UPROPERTY(BlueprintReadWrite, Category = CameraVolumes)
		bool bUpdateCamera;

#if WITH_EDITOR
	//Override PostEditChangeProperty
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	UPROPERTY()
		bool bIsCameraOrthographic;
};
