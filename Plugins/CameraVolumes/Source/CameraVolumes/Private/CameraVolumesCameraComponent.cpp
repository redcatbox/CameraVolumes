// redbox, 2024

#include "CameraVolumesCameraComponent.h"
#include "CameraVolumesFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CameraVolumesCameraComponent)

UCameraVolumesCameraComponent::UCameraVolumesCameraComponent()
{
	// Camera defaults
	DefaultCameraLocation = FVector(0.f, 1000.f, 0.f);
	DefaultCameraFieldOfView = 90.f;
	DefaultCameraOrthoWidth = 512.f;

	// Camera lag
	CameraLocationLagSpeed = 10.0f;
	CameraRotationLagSpeed = 10.0f;
	CameraFOVInterpSpeed = 10.f;
	CameraOrthoWidthInterpSpeed = 10.f;

	// Camera collision
	ProbeSize = 12.f;
	ProbeChannel = ECC_Camera;

	// Camera rotation
	bUpdateControlRotationFromCameraRotation = true;
	bInheritPitchCV = true;
	bInheritYawCV = true;
	bInheritRollCV = true;

	bUpdateCamera = true;

	LoadConfig();

	UpdateCameraComponent();
}

void UCameraVolumesCameraComponent::UpdateCamera(FMinimalViewInfo& InViewInfo)
{
	if (bUpdateCamera)
	{
		SetWorldLocationAndRotation(InViewInfo.Location, InViewInfo.Rotation);
		SetOrthoWidth(InViewInfo.OrthoWidth);
		SetFieldOfView(InViewInfo.FOV);
	}
}

void UCameraVolumesCameraComponent::UpdateCameraComponent()
{
	switch (ProjectionMode)
	{
	case ECameraProjectionMode::Orthographic:
		SetOrthoWidth(DefaultCameraOrthoWidth);
		break;
	default:
		SetFieldOfView(DefaultCameraFieldOfView);
		break;
	}

	DefaultCameraRotation = UCameraVolumesFunctionLibrary::CalculateCameraRotation(DefaultCameraLocation, DefaultCameraFocalPoint, DefaultCameraRoll);
	SetRelativeLocationAndRotation(DefaultCameraLocation, DefaultCameraRotation);

#if WITH_EDITOR
	RefreshVisualRepresentation();
#endif
}

//Update with changed property
#if WITH_EDITOR
void UCameraVolumesCameraComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == TEXT("ProjectionMode")
		|| TEXT("DefaultCameraLocation")
		|| TEXT("DefaultCameraFocalPoint")
		|| TEXT("DefaultCameraRoll")
		|| TEXT("DefaultCameraFieldOfView") || TEXT("DefaultCameraOrthoWidth")
		)
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
