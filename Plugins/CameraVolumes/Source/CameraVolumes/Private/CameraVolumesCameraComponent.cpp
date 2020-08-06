// redbox, 2019

#include "CameraVolumesCameraComponent.h"
#include "CameraVolumesFunctionLibrary.h"

UCameraVolumesCameraComponent::UCameraVolumesCameraComponent()
{
	// Camera defaults
	DefaultCameraLocation = FVector(0.f, 1000.f, 0.f);
	DefaultCameraFocalPoint = FVector::ZeroVector;
	DefaultCameraRoll = 0.f;
	DefaultCameraFieldOfView = 90.f;
	DefaultCameraOrthoWidth = 512.f;
	bIsCameraOrthographic = false;
	UCameraVolumesCameraComponent::UpdateCameraComponent();

	// Camera lag
	bEnableCameraLocationLag = false;
	CameraLocationLagSpeed = 10.0f;
	bEnableCameraRotationLag = false;
	CameraRotationLagSpeed = 10.0f;
	bEnableCameraFOVInterp = false;
	CameraFOVInterpSpeed = 10.f;
	bEnableCameraOrthoWidthInterp = false;
	CameraOrthoWidthInterpSpeed = 10.f;

	// Additional params
	bUseAdditionalCameraParams = false;
	AdditionalCameraLocation = FVector::ZeroVector;
	AdditionalCameraRotation = FRotator::ZeroRotator;
	AdditionalCameraFOV = 0.f;
	AdditionalCameraOrthoWidth = 0.f;

	// Dead zone
	bUseDeadZone = false;
	DeadZoneExtent = FVector2D::ZeroVector;
	DeadZoneOffset = FVector2D::ZeroVector;
	bOverrideDeadZoneFocalPoint = false;
	OverridenDeadZoneFocalPoint = FVector::ZeroVector;

	// Camera collision
	bDoCollisionTest = false;
	ProbeSize = 12.0f;
	ProbeChannel = ECC_Camera;

	// Camera rotation
	bUsePawnControlRotationCV = false;
	bInheritPitchCV = true;
	bInheritYawCV = true;
	bInheritRollCV = true;

	bUpdateCamera = true;
}

void UCameraVolumesCameraComponent::UpdateCameraComponent()
{
	switch (ProjectionMode)
	{
	case ECameraProjectionMode::Orthographic:
		bIsCameraOrthographic = true;
		SetOrthoWidth(DefaultCameraOrthoWidth);
		break;
	case ECameraProjectionMode::Perspective:
		bIsCameraOrthographic = false;
		SetFieldOfView(DefaultCameraFieldOfView);
		break;
	default:
		bIsCameraOrthographic = false;
		SetFieldOfView(DefaultCameraFieldOfView);
		break;
	}

	DefaultCameraRotation = UCameraVolumesFunctionLibrary::CalculateCameraRotation(DefaultCameraLocation, DefaultCameraFocalPoint, DefaultCameraRoll);
	SetRelativeLocationAndRotation(DefaultCameraLocation, DefaultCameraRotation);
}

void UCameraVolumesCameraComponent::UpdateCamera(FVector& CameraLocation, FVector& CameraFocalPoint, FQuat& CameraRotation, float CameraFOV_OW, bool bIsCameraStatic)
{
	if (bUpdateCamera)
	{
		if (bDoCollisionTest && !bIsCameraStatic)
		{
			FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SpringArm), false, GetOwner());
			FHitResult HitResult;
			GetWorld()->SweepSingleByChannel(HitResult, CameraFocalPoint, CameraLocation, FQuat::Identity, ProbeChannel, FCollisionShape::MakeSphere(ProbeSize), QueryParams);

			if (HitResult.bBlockingHit)
			{
				CameraLocation = HitResult.Location;
			}
		}

		SetWorldLocationAndRotation(CameraLocation, CameraRotation);


		if (bIsCameraOrthographic)
		{
			SetOrthoWidth(CameraFOV_OW);
		}
		else
		{
			SetFieldOfView(CameraFOV_OW);
		}
	}
}

//Update with changed property
#if WITH_EDITOR
void UCameraVolumesCameraComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == TEXT("ProjectionMode")
		|| TEXT("DefaultCameraLocation")
		|| TEXT("DefaultCameraFocalPoint")
		|| TEXT("DefaultCameraRoll")
		|| TEXT("DefaultCameraFieldOfView")
		|| TEXT("DefaultCameraOrthoWidth"))
	{
		UpdateCameraComponent();
	}
}
#endif

// Runtime setters
void UCameraVolumesCameraComponent::SetDefaultCameraLocation(FVector NewDefaultCameraLocation)
{
	DefaultCameraLocation = NewDefaultCameraLocation;
	DefaultCameraRotation = UCameraVolumesFunctionLibrary::CalculateCameraRotation(DefaultCameraLocation, DefaultCameraFocalPoint, DefaultCameraRoll);
}

void UCameraVolumesCameraComponent::SetDefaultCameraFocalPoint(FVector NewDefaultCameraFocalPoint)
{
	DefaultCameraFocalPoint = NewDefaultCameraFocalPoint;
	DefaultCameraRotation = UCameraVolumesFunctionLibrary::CalculateCameraRotation(DefaultCameraLocation, DefaultCameraFocalPoint, DefaultCameraRoll);
}

void UCameraVolumesCameraComponent::SetDefaultCameraRoll(float NewDefaultCameraRoll)
{
	DefaultCameraRoll = NewDefaultCameraRoll;
	DefaultCameraRotation = UCameraVolumesFunctionLibrary::CalculateCameraRotation(DefaultCameraLocation, DefaultCameraFocalPoint, DefaultCameraRoll);
}
