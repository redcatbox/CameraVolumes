// Dmitriy Barannik aka redbox, 2019

#include "CameraVolumesCameraComponent.h"
#include "CameraVolumesFunctionLibrary.h"

UCameraVolumesCameraComponent::UCameraVolumesCameraComponent()
{
	// Camera defaults
	DefaultCameraLocation = FVector(1000.f, 0.f, 0.f);
	DefaultCameraFocalPoint = FVector::ZeroVector;
	DefaultCameraRoll = 0.f;
	DefaultCameraFieldOfView = 90.f;
	UpdateCameraComponent();

	// Camera lag
	bEnableCameraLocationLag = false;
	CameraLocationLagSpeed = 10.0f;
	bEnableCameraRotationLag = false;
	CameraRotationLagSpeed = 10.0f;
	bEnableCameraFOVInterpolation = false;
	CameraFOVInterpolationSpeed = 10.f;
}

void UCameraVolumesCameraComponent::UpdateCameraComponent()
{
	DefaultCameraRotation = UCameraVolumesFunctionLibrary::CalculateCameraRotation(DefaultCameraLocation, DefaultCameraFocalPoint, DefaultCameraRoll);
	this->SetRelativeLocationAndRotation(DefaultCameraLocation, DefaultCameraRotation);
	this->SetFieldOfView(DefaultCameraFieldOfView);
}

void UCameraVolumesCameraComponent::UpdateCamera(FVector& CameraLocation, FQuat& CameraRotation, float CameraFOV)
{
	this->SetWorldLocation(CameraLocation);
	this->SetWorldRotation(CameraRotation);
	this->SetFieldOfView(CameraFOV);
}

//Update with changed property
#if WITH_EDITOR
void UCameraVolumesCameraComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == TEXT("DefaultCameraLocation") || TEXT("DefaultCameraFocalPoint") || TEXT("DefaultCameraRoll") || TEXT("DefaultCameraFieldOfView"))
		UpdateCameraComponent();
}
#endif