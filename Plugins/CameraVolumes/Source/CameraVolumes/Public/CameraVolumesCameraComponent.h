// Dmitriy Barannik aka redbox, 2019

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "CameraVolumeActor.h"
#include "CameraVolumesCameraComponent.generated.h"

UCLASS(AutoExpandCategories = (DefaultCameraParameters))
class CAMERAVOLUMES_API UCameraVolumesCameraComponent : public UCameraComponent
{
	GENERATED_BODY()
	
public:
	UCameraVolumesCameraComponent();

	/** Default camera RELATIVE location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters")
		FVector DefaultCameraLocation;

	/** Default camera RELATIVE focal point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters")
		FVector DefaultCameraFocalPoint;

	/** Default camera roll */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings | DefaultParameters")
		float DefaultCameraRoll;

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

	/** Overlapping camera volumes */
	UPROPERTY()
		TArray<ACameraVolumeActor*> OverlappingCameraVolumes;

	/** Updates camera by camera manager */
	UFUNCTION()
		virtual void UpdateCamera(FVector& CameraLocation, FQuat& CameraRotation, float CameraFOV);

	/** Get is camera uses orthographic projection mode */
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual bool GetIsCameraOrthographic() { return bIsCameraOrthographic; }

	/** Update camera component parameters */
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual void UpdateCameraComponent();

#if WITH_EDITOR
	//Override PostEditChangeProperty
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	UPROPERTY()
		bool bIsCameraOrthographic;
};
