// redbox, 2026

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
	bEnableCameraLocationLag = false;
	CameraLocationLagSpeed = 10.0f;

	bEnableCameraRotationLag = false;
	CameraRotationLagSpeed = 10.0f;

	bUseCameraLagSubstepping = true;
	bDrawDebugLagMarkers = false;
	CameraLagMaxTimeStep = 1.f / 60.f;
	CameraLagMaxDistance = 0.f;
	bClampToMaxPhysicsDeltaTime = false;

	bEnableCameraFOVInterp = false;
	CameraFOVInterpSpeed = 10.f;

	bEnableCameraOrthoWidthInterp = false;
	CameraOrthoWidthInterpSpeed = 10.f;

	bUseAdditionalCameraParams = false;

	// Camera collision
	bDoCollisionTest = false;
	ProbeSize = 12.f;
	ProbeChannel = ECC_Camera;

	// Camera rotation
	bUpdateControlRotationFromCameraRotation = true;
	bUsePawnControlRotationCV = false;
	bInheritPitchCV = true;
	bInheritYawCV = true;
	bInheritRollCV = true;

	bUpdateCamera = true;

	LoadConfig();

#if WITH_EDITOR
	UpdateCameraComponent();
#endif
}

void UCameraVolumesCameraComponent::OnRegister()
{
	DefaultCameraRotation = UCameraVolumesFunctionLibrary::CalculateCameraRotation(DefaultCameraLocation, DefaultCameraFocalPoint, DefaultCameraRoll);
	SetRelativeLocationAndRotation(DefaultCameraLocation, DefaultCameraRotation);

	Super::OnRegister();
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

#if WITH_EDITOR
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

	RefreshVisualRepresentation();
}
#endif

// Update with changed property
#if WITH_EDITOR
void UCameraVolumesCameraComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCameraVolumesCameraComponent, ProjectionMode)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UCameraVolumesCameraComponent, DefaultCameraLocation)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UCameraVolumesCameraComponent, DefaultCameraFocalPoint)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UCameraVolumesCameraComponent, DefaultCameraRoll)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UCameraVolumesCameraComponent, DefaultCameraFieldOfView)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UCameraVolumesCameraComponent, DefaultCameraOrthoWidth)
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
