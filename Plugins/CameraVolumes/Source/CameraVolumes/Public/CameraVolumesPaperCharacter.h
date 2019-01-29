//Dmitriy Barannik aka redbox, 2019

/**
* Player character stores default camera parameters and camera component.
* Player character uses BeginOverlap event of it's capsule collision primitive, to get overlapped camera volume and store it in special array of camera volume actors.
* Player character uses EndOverlap event of it's capsule collision primitive, to remove camera volume from array of camera volume actors.
*/

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "GameFramework/PlayerController.h"
#include "CameraVolumeActor.h"
#include "CameraVolumesPaperCharacter.generated.h"

UCLASS(AutoExpandCategories = (Camera))
class CAMERAVOLUMES_API ACameraVolumesPaperCharacter : public APaperCharacter
{
	GENERATED_BODY()

	/** Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* CameraComponent;

	UFUNCTION()
		void OnCapsuleComponentBeginOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnCapsuleComponentEndOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	ACameraVolumesPaperCharacter();
	virtual void PostInitializeComponents() override;

	/** Default camera RELATIVE location */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DefaultCameraParameters", Meta = (MakeEditWidget = true))
		FVector DefaultCameraLocation;

	/** Default camera RELATIVE focal point */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DefaultCameraParameters", Meta = (MakeEditWidget = true))
		FVector DefaultCameraFocalPoint;

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

	/** Updates camera by camera manager*/
	UFUNCTION()
		virtual void UpdateCamera(FVector& CameraLocation, FQuat& CameraRotation, float CameraFOV);

	/** Overlapping camera volumes */
	UPROPERTY()
		TArray<ACameraVolumeActor*> OverlappingCameraVolumes;

	/** Returns SideViewCameraComponent subobject */
	FORCEINLINE class UCameraComponent* GetCameraComponent() const { return CameraComponent; }

	//Override PostEditChangeProperty
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	UFUNCTION()
		virtual void UpdateCameraComponent();
};
