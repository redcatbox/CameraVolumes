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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DefaultCameraParameters", Meta = (MakeEditWidget = true))
		FVector DefaultCameraLocation;

	/** Default camera RELATIVE focal point */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DefaultCameraParameters", Meta = (MakeEditWidget = true))
		FVector DefaultCameraFocalPoint;

	/** Default camera roll */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DefaultCameraParameters")
		float DefaultCameraRoll;

	UPROPERTY()
		FQuat DefaultCameraRotation;

	/** Default camera FOV */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DefaultCameraParameters", Meta = (UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0", Units = deg))
		float DefaultCameraFieldOfView;

	/** Should camera use location lag? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DefaultCameraParameters")
		bool bEnableCameraLocationLag;

	/** Camera location lag speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DefaultCameraParameters", Meta = (EditCondition = "bEnableCameraLocationLag", ClampMin = "0.1", ClampMax = "100.0", UIMin = "0.1", UIMax = "100.0"))
		float CameraLocationLagSpeed;

	/** Should camera use rotation lag? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DefaultCameraParameters")
		bool bEnableCameraRotationLag;

	/** Camera rotation lag speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DefaultCameraParameters", Meta = (EditCondition = "bEnableCameraRotationLag", ClampMin = "0.1", ClampMax = "100.0", UIMin = "0.1", UIMax = "100.0"))
		float CameraRotationLagSpeed;

	/** Should camera use FOV interpolation? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DefaultCameraParameters")
		bool bEnableCameraFOVInterpolation;

	/** Camera FOV Interpolation speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DefaultCameraParameters", Meta = (EditCondition = "bEnableCameraFOVInterpolation", ClampMin = "0.1", ClampMax = "100.0", UIMin = "0.1", UIMax = "100.0"))
		float CameraFOVInterpolationSpeed;

	/** Overlapping camera volumes */
	UPROPERTY()
		TArray<ACameraVolumeActor*> OverlappingCameraVolumes;

	/** Updates camera by camera manager*/
	UFUNCTION()
		virtual void UpdateCamera(FVector& CameraLocation, FQuat& CameraRotation, float CameraFOV);

	//Override PostEditChangeProperty
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	UFUNCTION()
		virtual void UpdateCameraComponent();
};
