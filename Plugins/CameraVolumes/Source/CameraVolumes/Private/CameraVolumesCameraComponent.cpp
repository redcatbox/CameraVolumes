// redbox, 2021

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
#if WITH_EDITORONLY_DATA
	bPreviewDeadZone = false;
	DeadZonePreviewMaterialPath = TEXT("/CameraVolumes/Materials/DeadZonePreview");
#endif

	// Camera collision
	bDoCollisionTest = false;
	ProbeSize = 12.f;
	ProbeChannel = ECC_Camera;

	// Camera rotation
	bUsePawnControlRotationCV = false;
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
	UpdateDeadZonePreview(DeadZoneExtent, DeadZoneOffset);
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
void UCameraVolumesCameraComponent::UpdateDeadZonePreview(FVector2D& NewDeadZoneExtent, FVector2D& NewDeadZoneOffset)
{
	if (bPreviewDeadZone)
	{
		if (!DeadZonePreviewMID)
		{
			DeadZonePreviewMID = UMaterialInstanceDynamic::Create(DeadZonePreviewMaterial, this);
			AddOrUpdateBlendable(DeadZonePreviewMID, 1.f);
		}

		DeadZonePreviewMID->SetScalarParameterValue(FName(TEXT("Size_X")), NewDeadZoneExtent.X);
		DeadZonePreviewMID->SetScalarParameterValue(FName(TEXT("Size_Y")), NewDeadZoneExtent.Y);
		DeadZonePreviewMID->SetScalarParameterValue(FName(TEXT("Offset_X")), NewDeadZoneOffset.X);
		DeadZonePreviewMID->SetScalarParameterValue(FName(TEXT("Offset_Y")), NewDeadZoneOffset.Y);
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
