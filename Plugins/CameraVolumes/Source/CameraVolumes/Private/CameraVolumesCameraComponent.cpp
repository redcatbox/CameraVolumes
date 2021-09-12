// redbox, 2021

#include "CameraVolumesCameraComponent.h"
#include "CameraVolumesFunctionLibrary.h"

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

	bUpdateControlRotationFromCameraRotation = true;

	// Dead zone
#if WITH_EDITORONLY_DATA
	DeadZonePreviewMaterialPath = TEXT("/CameraVolumes/Materials/DeadZonePreview");
#endif

	// Camera collision
	ProbeSize = 12.f;
	ProbeChannel = ECC_Camera;

	// Camera rotation
	bInheritPitchCV = true;
	bInheritYawCV = true;
	bInheritRollCV = true;

	bUpdateCamera = true;

	LoadConfig();

#if WITH_EDITORONLY_DATA
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialObj(*DeadZonePreviewMaterialPath);
	DeadZonePreviewMaterial = MaterialObj.Object;
#endif

	UCameraVolumesCameraComponent::UpdateCameraComponent();
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

void UCameraVolumesCameraComponent::UpdateCameraComponent()
{
	switch (ProjectionMode)
	{
	case ECameraProjectionMode::Orthographic:
		bIsCameraOrthographic = true;
		SetOrthoWidth(DefaultCameraOrthoWidth);
		break;
	default:
		bIsCameraOrthographic = false;
		SetFieldOfView(DefaultCameraFieldOfView);
		break;
	}

	DefaultCameraRotation = UCameraVolumesFunctionLibrary::CalculateCameraRotation(DefaultCameraLocation, DefaultCameraFocalPoint, DefaultCameraRoll);
	SetRelativeLocationAndRotation(DefaultCameraLocation, DefaultCameraRotation);

#if WITH_EDITOR
	FDeadZoneTransform DeadZoneTransform(DeadZoneExtent, DeadZoneOffset, DefaultCameraRoll);
	UpdateDeadZonePreview(DeadZoneTransform);
#endif

#if WITH_EDITORONLY_DATA
	RefreshVisualRepresentation();
#endif
}

bool UCameraVolumesCameraComponent::GetIsCameraOrthographic() const
{
	return bIsCameraOrthographic;
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
		|| TEXT("bUseDeadZone") || TEXT("DeadZoneExtent") || TEXT("DeadZoneOffset") || TEXT("bPreviewDeadZone"))
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

#if WITH_EDITOR
void UCameraVolumesCameraComponent::UpdateDeadZonePreview(FDeadZoneTransform& NewDeadZoneTransform)
{
	if (bPreviewDeadZone)
	{
		if (!DeadZonePreviewMID)
		{
			DeadZonePreviewMID = UMaterialInstanceDynamic::Create(DeadZonePreviewMaterial, this);
			AddOrUpdateBlendable(DeadZonePreviewMID, 1.f);
		}

		DeadZonePreviewMID->SetScalarParameterValue(FName(TEXT("Size_X")), NewDeadZoneTransform.DeadZoneExtent.X);
		DeadZonePreviewMID->SetScalarParameterValue(FName(TEXT("Size_Y")), NewDeadZoneTransform.DeadZoneExtent.Y);
		DeadZonePreviewMID->SetScalarParameterValue(FName(TEXT("Offset_X")), NewDeadZoneTransform.DeadZoneOffset.X);
		DeadZonePreviewMID->SetScalarParameterValue(FName(TEXT("Offset_Y")), NewDeadZoneTransform.DeadZoneOffset.Y);
		DeadZonePreviewMID->SetScalarParameterValue(FName(TEXT("Roll")), NewDeadZoneTransform.DeadZoneRoll);
	}
	else
	{
		if (DeadZonePreviewMID)
		{
			RemoveBlendable(DeadZonePreviewMID);
			DeadZonePreviewMID = nullptr;
		}
	}
}
#endif
