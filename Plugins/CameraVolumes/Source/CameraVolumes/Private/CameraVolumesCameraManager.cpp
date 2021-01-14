//redbox, 2021

#include "CameraVolumesCameraManager.h"
#include "CameraVolumesCharacterInterface.h"
#include "CameraVolumeDynamicActor.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"

ACameraVolumesCameraManager::ACameraVolumesCameraManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUpdateCamera = true;
	bCheckCameraVolumes = true;
	bPerformBlockingCalculations = true;
	PlayerPawnLocationOld = FVector::ZeroVector;
	PlayerPawnLocation = FVector::ZeroVector;
	CameraVolumePrevious = nullptr;
	CameraVolumeCurrent = nullptr;
	CameraLocationOld = FVector::ZeroVector;
	CameraLocationNew = FVector::ZeroVector;
	CameraFocalPointNew = FVector::ZeroVector;
	CameraRotationOld = FQuat();
	CameraRotationNew = FQuat();
	CameraFOVOWOld = 90.f;
	CameraFOVOWNew = 90.f;
	bCameraComponentUseDeadZone = false;
	bCameraVolumeOverrideDeadZone = false;
	DeadZoneExtent = FVector2D();
	DeadZoneOffset = FVector2D();
	DeadZoneWorldCenterOld = FVector::ZeroVector;
	DeadZoneWorldCenterNew = FVector::ZeroVector;
	bIsCameraStatic = false;
	bIsCameraOrthographic = false;
	bNeedsSmoothTransition = false;
	bNeedsCutTransition = false;
	bBroadcastOnCameraVolumeChanged = false;
	bUsePlayerPawnControlRotation = false;
	LoadConfig();
}

void ACameraVolumesCameraManager::SetUpdateCamera(bool bNewUpdateCamera)
{
	bUpdateCamera = bNewUpdateCamera;
}

void ACameraVolumesCameraManager::SetCheckCameraVolumes(bool bNewCheck)
{
	if (bCheckCameraVolumes != bNewCheck)
	{
		bCheckCameraVolumes = bNewCheck;
		if (!bCheckCameraVolumes)
		{
			APawn* OwningPawn = GetOwningPlayerController()->GetPawn();
			if (OwningPawn)
			{
				ICameraVolumesCharacterInterface* PlayerCharacter = Cast<ICameraVolumesCharacterInterface>(PlayerPawn);
				if (PlayerCharacter)
				{
					UCameraVolumesCameraComponent* CamComp = PlayerCharacter->GetCameraComponent();
					CamComp->OverlappingCameraVolumes.Empty();
				}
			}
		}
	}
}

void ACameraVolumesCameraManager::SetPerformBlockingCalculations(bool bNewPerformBlockingCalculations)
{
	bPerformBlockingCalculations = bNewPerformBlockingCalculations;
}

void ACameraVolumesCameraManager::UpdateCamera(float DeltaTime)
{
	if (bUpdateCamera)
	{
		PlayerPawn = GetOwningPlayerController()->GetPawn();
		if (PlayerPawn)
		{
			ICameraVolumesCharacterInterface* PlayerCharacter = Cast<ICameraVolumesCharacterInterface>(PlayerPawn);
			if (PlayerCharacter)
			{
				//Store previous values and prepare params
				CameraVolumePrevious = CameraVolumeCurrent;
				CameraVolumeCurrent = nullptr;
				PlayerPawnLocationOld = PlayerPawnLocation;
				PlayerPawnLocation = PlayerPawn->GetActorLocation();
				CameraComponent = PlayerCharacter->GetCameraComponent();

				if (bCheckCameraVolumes)
				{
					// Check if camera component not contains any overlapped camera volumes,
					// try to get them from actors overlapping character's primitive component
					// and put into camera component
					if (CameraComponent->OverlappingCameraVolumes.Num() == 0)
					{
						TArray<AActor*> OverlappingActors;
						PlayerCharacter->GetCollisionPrimitiveComponent()->GetOverlappingActors(OverlappingActors, ACameraVolumeActor::StaticClass());

						if (OverlappingActors.Num() > 0)
						{
							for (AActor* OverlappingActor : OverlappingActors)
							{
								if (ACameraVolumeActor* CameraVolume = Cast<ACameraVolumeActor>(OverlappingActor))
								{
									CameraComponent->OverlappingCameraVolumes.AddUnique(CameraVolume);
								}
							}

							CameraVolumeCurrent = UCameraVolumesFunctionLibrary::GetCurrentCameraVolume(CameraComponent->OverlappingCameraVolumes, PlayerPawnLocation);
						}
						else
						{
							// There are no camera volumes overlapping character at this time,
							// so we don't need this check until player pawn overlap some camera volume again.
							SetCheckCameraVolumes(false);
						}
					}
					else
					{
						CameraVolumeCurrent = UCameraVolumesFunctionLibrary::GetCurrentCameraVolume(CameraComponent->OverlappingCameraVolumes, PlayerPawnLocation);
					}

					if (CameraVolumeCurrent)
					{
						if (CameraVolumeCurrent != CameraVolumePrevious) // Do we changed to another volume?
						{
							const FSideInfo PassedSideInfoCurrent = CameraVolumeCurrent->GetNearestVolumeSideInfo(PlayerPawnLocation);

							if (CameraVolumePrevious)
							{
								const FSideInfo PassedSideInfoPrevious = CameraVolumePrevious->GetNearestVolumeSideInfo(PlayerPawnLocation);

								if (UCameraVolumesFunctionLibrary::CompareSidesPairs(PassedSideInfoCurrent.Side, PassedSideInfoPrevious.Side, CameraVolumePrevious->bUse6DOFVolume))
								{
									// We've passed to nearby volume
									// Use settings of side we have passed to
									SetTransitionBySideInfo(CameraVolumeCurrent, PassedSideInfoCurrent);
								}
								else
								{
									// We've passed to volume with another priority
									if (CameraVolumeCurrent->Priority > CameraVolumePrevious->Priority)
									{
										// Use settings of side we have passed to
										SetTransitionBySideInfo(CameraVolumeCurrent, PassedSideInfoCurrent);
									}
									else
									{
										// Use settings of side we have passed from
										SetTransitionBySideInfo(CameraVolumePrevious, PassedSideInfoPrevious);
									}
								}
							}
							else
							{
								// We've passed from void to volume
								SetTransitionBySideInfo(CameraVolumeCurrent, PassedSideInfoCurrent);
							}
						}

						CalcNewCameraParams(CameraVolumeCurrent, DeltaTime);
					}
					else if (CameraVolumePrevious) // Do we passed from volume to void?
					{
						// Use settings of side we've passed from
						const FSideInfo PassedSideInfoPrevious = CameraVolumePrevious->GetNearestVolumeSideInfo(PlayerPawnLocation);
						SetTransitionBySideInfo(CameraVolumePrevious, PassedSideInfoPrevious);
						CalcNewCameraParams(nullptr, DeltaTime);
					}
					else
					{
						CalcNewCameraParams(nullptr, DeltaTime);
					}
				}
				else
				{
					CalcNewCameraParams(nullptr, DeltaTime);
				}

				// Update camera component
				CameraComponent->UpdateCamera(CameraLocationFinalNew, CameraFocalPointNew, CameraRotationFinalNew, CameraFOVOWFinalNew, bIsCameraStatic);

				// Update camera manager
				if (bIsCameraOrthographic)
				{
					DefaultOrthoWidth = CameraFOVOWFinalNew;
				}
				else
				{
					DefaultFOV = CameraFOVOWFinalNew;
				}
			}
		}
	}

	Super::UpdateCamera(DeltaTime);
}

void ACameraVolumesCameraManager::CalcNewCameraParams(ACameraVolumeActor* CameraVolume, float DeltaTime)
{
	//Store previous values and prepare params
	CameraLocationOld = CameraLocationNew;
	CameraRotationOld = CameraRotationNew;
	CameraLocationNew = PlayerPawnLocation + CameraComponent->DefaultCameraLocation;
	CameraRotationNew = CameraComponent->DefaultCameraRotation;
	CameraFocalPointNew = PlayerPawnLocation + CameraComponent->DefaultCameraFocalPoint;
	bIsCameraStatic = false;
	bIsCameraOrthographic = CameraComponent->GetIsCameraOrthographic();
	CameraFOVOWOld = CameraFOVOWNew;
	bBroadcastOnCameraVolumeChanged = false;
	bUsePlayerPawnControlRotation = false;
	CameraFOVOWNew = bIsCameraOrthographic
		? CameraComponent->DefaultCameraOrthoWidth
		: CameraComponent->DefaultCameraFieldOfView;
	bCameraComponentUseDeadZone = false;
	bCameraVolumeOverrideDeadZone = false;
	DeadZoneWorldCenterNew = DeadZoneWorldCenterOld;

	if (CameraVolume)
	{
		if (bIsCameraOrthographic)
		{
			if (CameraVolume->bOverrideCameraOrthoWidth)
			{
				CameraFOVOWNew = CameraVolume->CameraOrthoWidth;
			}
		}
		else
		{
			if (CameraVolume->bOverrideCameraFieldOfView)
			{
				CameraFOVOWNew = CameraVolume->CameraFieldOfView;
			}
		}

		// Get screen (or at least player camera) aspect ratio for further calculations
		const float CamFOVTangens = FMath::Tan(FMath::DegreesToRadians(CameraFOVOWNew * 0.5f));
		float ScreenAspectRatio = CameraComponent->AspectRatio;

		if (GEngine->GameViewport->Viewport)
		{
			const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
			if (ViewportSize.Y != 0)
			{
				ScreenAspectRatio = ViewportSize.X / ViewportSize.Y;
			}
		}

		// Location and Rotation
		const FVector PlayerPawnLocationTransformed = CameraVolume->GetActorTransform().InverseTransformPositionNoScale(PlayerPawnLocation);

		bIsCameraStatic = CameraVolume->GetIsCameraStatic();
		if (bIsCameraStatic)
		{
			CameraLocationNew = CameraVolume->CameraLocation;

			if (CameraVolume->bFocalPointIsPlayer)
			{
				CameraFocalPointNew = PlayerPawnLocationTransformed + CameraVolume->CameraFocalPoint;
				CameraRotationNew = UCameraVolumesFunctionLibrary::CalculateCameraRotation(CameraVolume->CameraLocation, CameraFocalPointNew, CameraVolume->CameraRoll);
			}
			else
			{
				CameraRotationNew = CameraVolume->CameraRotation;
			}
		}
		else
		{
			if (CameraVolume->bUseCameraRotationAxis)
			{
				CameraLocationNew = CameraVolume->bOverrideCameraLocation
					? CameraVolume->CameraLocation
					: CameraComponent->DefaultCameraLocation;

				const FVector DirToAxis = PlayerPawnLocationTransformed.GetSafeNormal2D();
				const FQuat RotToAxis = FRotationMatrix::MakeFromX(DirToAxis).ToQuat();
				CameraLocationNew = RotToAxis.RotateVector(CameraLocationNew) + FVector(0.f, 0.f, PlayerPawnLocationTransformed.Z);

				if (!CameraVolume->bCameraLocationRelativeToVolume && CameraVolume->bOverrideCameraLocation)
				{
					CameraLocationNew += FVector(PlayerPawnLocationTransformed.X, PlayerPawnLocationTransformed.Y, 0.f);
				}

				CameraFocalPointNew = PlayerPawnLocationTransformed;
				CameraFocalPointNew += CameraVolume->bOverrideCameraRotation
					? RotToAxis.RotateVector(CameraVolume->CameraFocalPoint)
					: RotToAxis.RotateVector(CameraComponent->DefaultCameraFocalPoint);

				const float NewCameraRoll = CameraVolume->bOverrideCameraRotation
					? CameraVolume->CameraRoll
					: CameraComponent->DefaultCameraRoll;

				CameraRotationNew = UCameraVolumesFunctionLibrary::CalculateCameraRotation(CameraLocationNew, CameraFocalPointNew, NewCameraRoll);
			}
			else
			{
				CameraLocationNew = PlayerPawnLocationTransformed;
				CameraFocalPointNew = PlayerPawnLocationTransformed;

				CameraLocationNew += CameraVolume->bOverrideCameraLocation
					? CameraVolume->CameraLocation
					: CameraComponent->DefaultCameraLocation;

				CameraFocalPointNew += CameraVolume->bOverrideCameraRotation
					? CameraVolume->CameraFocalPoint
					: CameraComponent->DefaultCameraFocalPoint;

				if (CameraVolume->bOverrideCameraRotation)
				{
					CameraRotationNew = CameraVolume->CameraRotation;
				}

				if (CameraVolume->bCameraLocationRelativeToVolume)
				{
					CameraLocationNew.Y = CameraVolume->bOverrideCameraLocation
						? CameraVolume->CameraLocation.Y
						: CameraComponent->DefaultCameraLocation.Y;

					CameraFocalPointNew.Y = CameraVolume->bOverrideCameraRotation
						? CameraVolume->CameraFocalPoint.Y
						: CameraComponent->DefaultCameraFocalPoint.Y;
				}
			}

			// Calculate camera blocking like it oriented to volume Front side
			bBlockingCalculations = bPerformBlockingCalculations
				? CameraVolumeCurrent->bPerformCameraBlocking
				: bPerformBlockingCalculations;

			if (bBlockingCalculations)
			{
				FVector NewCamVolExtentCorrected = CameraVolume->GetCamVolExtentCorrected();
				FVector NewCamVolMinCorrected = CameraVolume->GetCamVolMinCorrected();
				FVector NewCamVolMaxCorrected = CameraVolume->GetCamVolMaxCorrected();

				// Calculate delta volume extent with max +Y volume coordinate
				if (!CameraVolume->bUseZeroDepthExtent || !bIsCameraOrthographic)
				{
					FVector DeltaExtent;
					DeltaExtent.X = FMath::Abs((NewCamVolMaxCorrected.Y) * CamFOVTangens);
					DeltaExtent = FVector(DeltaExtent.X, 0.f, DeltaExtent.X / ScreenAspectRatio);
					NewCamVolExtentCorrected += DeltaExtent;
					NewCamVolMinCorrected -= DeltaExtent;
					NewCamVolMaxCorrected += DeltaExtent;
				}

				// Camera offset is always relative to camera volume local Y axis
				float CameraOffset = CameraVolume->bUseCameraRotationAxis
					? CameraLocationNew.Size2D()
					: CameraLocationNew.Y;

				// Calculate new camera offset and screen world extent at depth of CameraOffset
				FVector ScreenExtent;

				if (bIsCameraOrthographic)
				{
					CameraFOVOWNew = CameraVolume->GetCamVolAspectRatio() >= ScreenAspectRatio
						? FMath::Clamp(CameraFOVOWNew, CameraFOVOWNew, 2.f * NewCamVolExtentCorrected.Z * ScreenAspectRatio)
						: FMath::Clamp(CameraFOVOWNew, CameraFOVOWNew, 2.f * NewCamVolExtentCorrected.X);

					ScreenExtent.X = CameraFOVOWNew * 0.5f;
				}
				else
				{
					if (CameraVolume->bUseCameraRotationAxis)
					{
						CameraOffset = FMath::Clamp(
							CameraOffset,
							CameraOffset,
							NewCamVolExtentCorrected.Z * ScreenAspectRatio / CamFOVTangens);
					}
					else
					{
						CameraOffset = CameraVolume->GetCamVolAspectRatio() >= ScreenAspectRatio
							? FMath::Clamp(
								CameraOffset,
								CameraOffset,
								NewCamVolExtentCorrected.Z * ScreenAspectRatio / CamFOVTangens)
							: FMath::Clamp(
								CameraOffset,
								CameraOffset,
								NewCamVolExtentCorrected.X / CamFOVTangens);
					}

					ScreenExtent.X = FMath::Abs(CameraOffset * CamFOVTangens);
				}

				ScreenExtent = FVector(ScreenExtent.X, 0.f, ScreenExtent.X / ScreenAspectRatio);
				const FVector ScreenMin = NewCamVolMinCorrected + ScreenExtent;
				const FVector ScreenMax = NewCamVolMaxCorrected - ScreenExtent;

				// Perform camera blocking only on top and bottom sides
				CameraLocationNew = CameraVolume->bUseCameraRotationAxis
					? FVector(
						CameraLocationNew.GetSafeNormal2D().X * CameraOffset,
						CameraLocationNew.GetSafeNormal2D().Y * CameraOffset,
						FMath::Clamp(CameraLocationNew.Z, ScreenMin.Z, ScreenMax.Z))
					: FVector(
						FMath::Clamp(CameraLocationNew.X, ScreenMin.X, ScreenMax.X),
						CameraOffset,
						FMath::Clamp(CameraLocationNew.Z, ScreenMin.Z, ScreenMax.Z));
			}
		}

		// Final world-space values
		CameraLocationNew = CameraVolume->GetActorTransform().TransformPositionNoScale(CameraLocationNew);
		CameraFocalPointNew = CameraVolume->GetActorTransform().TransformPositionNoScale(CameraFocalPointNew);
		CameraRotationNew = CameraVolume->GetActorTransform().TransformRotation(CameraRotationNew);

		bUsePlayerPawnControlRotation = false;

		// Get dead zone params
		bCameraVolumeOverrideDeadZone = CameraVolume->bOverrideDeadZoneSettings;
		if (bCameraVolumeOverrideDeadZone)
		{
			DeadZoneExtent = CameraVolume->DeadZoneExtent;
			DeadZoneOffset = CameraVolume->DeadZoneOffset;
		}
	}
	else if (CameraComponent->bUsePawnControlRotationCV)
	{
		const FRotator CamRot = CameraComponent->DefaultCameraRotation.Rotator();
		FRotator PawnViewRot = PlayerPawn->GetViewRotation();

		if (!CameraComponent->bInheritPitchCV)
		{
			PawnViewRot.Pitch = CamRot.Pitch;
		}

		if (!CameraComponent->bInheritYawCV)
		{
			PawnViewRot.Yaw = CamRot.Yaw;
		}

		if (!CameraComponent->bInheritRollCV)
		{
			PawnViewRot.Roll = CamRot.Roll;
		}

		CameraRotationNew = PawnViewRot.Quaternion();

		if (CameraComponent->bEnableCameraRotationLag)
		{
			CameraRotationNew = FMath::QInterpTo(CameraRotationOld, CameraRotationNew, DeltaTime, CameraComponent->CameraRotationLagSpeed);
		}

		CameraLocationNew = PlayerPawnLocation;

		if (CameraComponent->bEnableCameraLocationLag)
		{
			CameraLocationNew = FMath::VInterpTo(PlayerPawnLocationOld, CameraLocationNew, DeltaTime, CameraComponent->CameraLocationLagSpeed);
		}

		PlayerPawnLocationOld = CameraLocationNew;
		CameraFocalPointNew = CameraLocationNew + PlayerPawn->GetActorQuat().RotateVector(CameraComponent->DefaultCameraFocalPoint);
		CameraLocationNew = CameraFocalPointNew + CameraRotationNew.RotateVector(CameraComponent->DefaultCameraLocation - CameraComponent->DefaultCameraFocalPoint);

		bUsePlayerPawnControlRotation = true;
	}

	// Transitions and interpolation
	if (bNeedsSmoothTransition)
	{
		SmoothTransitionAlpha += DeltaTime * SmoothTransitionSpeed;

		if (SmoothTransitionAlpha <= 1.f)
		{
			CameraLocationNew = FMath::Lerp(CameraLocationOld, CameraLocationNew, SmoothTransitionAlpha);
			CameraRotationNew = FQuat::Slerp(CameraRotationOld, CameraRotationNew, SmoothTransitionAlpha);
			CameraFOVOWNew = FMath::Lerp(CameraFOVOWOld, CameraFOVOWNew, SmoothTransitionAlpha);
		}
		else
		{
			SmoothTransitionAlpha = 0.f;
			bNeedsSmoothTransition = false;
		}
	}
	else if (bNeedsCutTransition)
	{
		bNeedsSmoothTransition = false;
		bNeedsCutTransition = false;
		bGameCameraCutThisFrame = true;
	}
	else
	{
		if (CameraComponent->bEnableCameraLocationLag && !bUsePlayerPawnControlRotation)
		{
			CameraLocationNew = FMath::VInterpTo(CameraLocationOld, CameraLocationNew, DeltaTime, CameraComponent->CameraLocationLagSpeed);
		}

		if (CameraComponent->bEnableCameraRotationLag && !bUsePlayerPawnControlRotation)
		{
			CameraRotationNew = FMath::QInterpTo(CameraRotationOld, CameraRotationNew, DeltaTime, CameraComponent->CameraRotationLagSpeed);
		}

		if (bIsCameraOrthographic)
		{
			if (CameraComponent->bEnableCameraOrthoWidthInterp)
			{
				CameraFOVOWNew = FMath::FInterpTo(CameraFOVOWOld, CameraFOVOWNew, DeltaTime, CameraComponent->CameraOrthoWidthInterpSpeed);
			}
		}
		else if (CameraComponent->bEnableCameraFOVInterp)
		{
			CameraFOVOWNew = FMath::FInterpTo(CameraFOVOWOld, CameraFOVOWNew, DeltaTime, CameraComponent->CameraFOVInterpSpeed);
		}
	}

	// Dead zone
	if (!bCameraVolumeOverrideDeadZone)
	{
		bCameraComponentUseDeadZone = CameraComponent->bUseDeadZone;
		if (bCameraComponentUseDeadZone)
		{
			DeadZoneExtent = CameraComponent->DeadZoneExtent;
			DeadZoneOffset = CameraComponent->DeadZoneOffset;
		}
	}

	if (bCameraComponentUseDeadZone || bCameraVolumeOverrideDeadZone)
	{
		CameraComponent->UpdateDeadZonePreview(DeadZoneExtent, DeadZoneOffset);
		
		const FVector DeadZoneFocalPoint = CameraComponent->GetOverrideDeadZoneFocalPoint()
			? CameraComponent->GetOverridenDeadZoneFocalPoint()
			: PlayerPawnLocation;

		if (IsInDeadZone(DeadZoneFocalPoint))
		{
		}

		CameraLocationFinalNew = CameraLocationNew;
		CameraRotationFinalNew = CameraRotationNew;
		CameraFOVOWFinalNew = CameraFOVOWNew;
	}
	else
	{
		CameraLocationFinalNew = CameraLocationNew;
		CameraRotationFinalNew = CameraRotationNew;
		CameraFOVOWFinalNew = CameraFOVOWNew;
	}

	// Additional camera params
	if (CameraComponent->bUseAdditionalCameraParams)
	{
		CameraLocationFinalNew += CameraComponent->AdditionalCameraLocation;
		CameraRotationFinalNew *= CameraComponent->AdditionalCameraRotation.Quaternion();
		CameraFOVOWFinalNew += bIsCameraOrthographic
			? CameraComponent->AdditionalCameraOrthoWidth
			: CameraComponent->AdditionalCameraFOV;
	}

	// Broadcast volume changed event
	if (bBroadcastOnCameraVolumeChanged)
	{
		OnCameraVolumeChanged.Broadcast(BroadcastCameraVolume, BroadcastSideInfo);
	}
}

void ACameraVolumesCameraManager::SetTransitionBySideInfo(ACameraVolumeActor* CameraVolume, FSideInfo SideInfo)
{
	if (SideInfo.SideTransitionType == ESideTransitionType::ESTT_Smooth)
	{
		bNeedsSmoothTransition = true;
		SmoothTransitionAlpha = 0.f;
		SmoothTransitionSpeed = CameraVolume->CameraSmoothTransitionSpeed;
	}
	else if (SideInfo.SideTransitionType == ESideTransitionType::ESTT_Cut)
	{
		bNeedsCutTransition = true;
		bNeedsSmoothTransition = false;
		SmoothTransitionAlpha = 0.f;
	}
	else
	{
		bNeedsSmoothTransition = false;
		SmoothTransitionAlpha = 0.f;
		bNeedsCutTransition = false;
	}

	// Store OnCameraVolumeChanged broadcast params
	bBroadcastOnCameraVolumeChanged = true;

	if (bBroadcastOnCameraVolumeChanged)
	{
		BroadcastCameraVolume = CameraVolume;
		BroadcastSideInfo = SideInfo;
	}
}

bool ACameraVolumesCameraManager::IsInDeadZone(FVector WorldLocationToCheck)
{
	if (APlayerController* PC = GetOwningPlayerController())
	{
		if (GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport && CameraComponent)
		{
			FVector2D ScreenLocation;
			PC->ProjectWorldLocationToScreen(WorldLocationToCheck, ScreenLocation);

			const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
			const FVector2D ScreenCenter = ViewportSize / 2;
			const FVector2D DeadZoneScreenOffset = ViewportSize * CameraComponent->DeadZoneOffset / 100.f;
			const FVector2D DeadZoneScreenExtent = ViewportSize * CameraComponent->DeadZoneExtent / 200.f;
			const FVector2D DeadZoneScreenCenter = ScreenCenter + DeadZoneScreenOffset;
			const FVector2D DeadZoneScreenMin = DeadZoneScreenCenter - DeadZoneScreenExtent;
			const FVector2D DeadZoneScreenMax = DeadZoneScreenCenter + DeadZoneScreenExtent;

			return (ScreenLocation >= DeadZoneScreenMin) && (ScreenLocation <= DeadZoneScreenMax);
		}
	}

	return false;
}

FVector2D ACameraVolumesCameraManager::CalculateScreenWorldExtentAtDepth(float Depth)
{
	FVector2D ScreenExtentResult = FVector2D::ZeroVector;
	float ScreenAspectRatio = 0.f;

	if (CameraComponent)
	{
		ScreenAspectRatio = CameraComponent->AspectRatio;

		ScreenExtentResult.X = CameraComponent->GetIsCameraOrthographic()
			? DefaultOrthoWidth * 0.5f
			: ScreenExtentResult.X = FMath::Abs(Depth * FMath::Tan(FMath::DegreesToRadians(DefaultFOV * 0.5f)));
	}

	if (GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport)
	{
		const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
		if (ViewportSize.Y != 0)
		{
			ScreenAspectRatio = ViewportSize.X / ViewportSize.Y;
		}
	}

	ScreenExtentResult = FVector2D(ScreenExtentResult.X, ScreenExtentResult.X / ScreenAspectRatio);
	return ScreenExtentResult;
}
