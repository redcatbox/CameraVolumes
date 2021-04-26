// redbox, 2021

#include "CameraVolumesCameraManager.h"
#include "CameraVolumesCharacterInterface.h"
#include "CameraVolumeDynamicActor.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"

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
	bFirstPass = true;
	bUseDeadZone = false;
	DeadZoneExtent = FVector2D::ZeroVector;
	DeadZoneOffset = FVector2D::ZeroVector;
	bIsInDeadZone = false;
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

						CalculateCameraParams(DeltaTime);
					}
					else if (CameraVolumePrevious) // Do we passed from volume to void?
					{
						// Use settings of side we've passed from
						const FSideInfo PassedSideInfoPrevious = CameraVolumePrevious->GetNearestVolumeSideInfo(PlayerPawnLocation);
						SetTransitionBySideInfo(CameraVolumePrevious, PassedSideInfoPrevious);
						CalculateCameraParams(DeltaTime);
					}
					else
					{
						CalculateCameraParams(DeltaTime);
					}
				}
				else
				{
					CalculateCameraParams(DeltaTime);
				}

				// Update camera component
				CameraComponent->UpdateCamera(CameraLocationFinalNew, CameraFocalPointNew, CameraRotationFinalNew, CameraFOVOWFinalNew, bIsCameraStatic);

#if WITH_EDITOR
				// Dead zone
				FDeadZoneTransform DeadZoneTransform(DeadZoneExtent, DeadZoneOffset, -CameraRotationFinalNew.Rotator().Roll);
				CameraComponent->UpdateDeadZonePreview(DeadZoneTransform);
#endif

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

void ACameraVolumesCameraManager::CalculateCameraParams(float DeltaTime)
{
	// Store previous values and prepare params
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
	bUseDeadZone = false;
	bIsInDeadZone = false;

	// Dead zone
	if (CameraComponent->bUseDeadZone)
	{
		bUseDeadZone = true;
		DeadZoneExtent = CameraComponent->DeadZoneExtent;
		DeadZoneOffset = CameraComponent->DeadZoneOffset;
	}

	if (CameraVolumeCurrent)
	{
		if (bIsCameraOrthographic)
		{
			if (CameraVolumeCurrent->bOverrideCameraOrthoWidth)
			{
				CameraFOVOWNew = CameraVolumeCurrent->CameraOrthoWidth;
			}
		}
		else
		{
			if (CameraVolumeCurrent->bOverrideCameraFieldOfView)
			{
				CameraFOVOWNew = CameraVolumeCurrent->CameraFieldOfView;
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
		const FVector PlayerPawnLocationTransformed = CameraVolumeCurrent->GetActorTransform().InverseTransformPositionNoScale(PlayerPawnLocation);

		// Dead zone
		if (CameraVolumeCurrent->bOverrideDeadZoneSettings)
		{
			bUseDeadZone = true;
			DeadZoneExtent = CameraVolumeCurrent->DeadZoneExtent;
			DeadZoneOffset = CameraVolumeCurrent->DeadZoneOffset;
		}

		bIsCameraStatic = CameraVolumeCurrent->GetIsCameraStatic();
		if (bIsCameraStatic)
		{
			CameraLocationNew = CameraVolumeCurrent->CameraLocation;

			if (CameraVolumeCurrent->bFocalPointIsPlayer)
			{
				CameraFocalPointNew = PlayerPawnLocationTransformed + CameraVolumeCurrent->CameraFocalPoint;
				CameraRotationNew = UCameraVolumesFunctionLibrary::CalculateCameraRotation(CameraVolumeCurrent->CameraLocation, CameraFocalPointNew, CameraVolumeCurrent->CameraRoll);
			}
			else
			{
				CameraRotationNew = CameraVolumeCurrent->CameraRotation;
				// Do not process dead zone for fully static camera
				bUseDeadZone = false;
			}
		}
		else
		{
			if (CameraVolumeCurrent->bUseCameraRotationAxis)
			{
				CameraLocationNew = CameraVolumeCurrent->bOverrideCameraLocation
					? CameraVolumeCurrent->CameraLocation
					: CameraComponent->DefaultCameraLocation;

				const FVector DirToAxis = PlayerPawnLocationTransformed.GetSafeNormal2D();
				const FQuat RotToAxis = FRotationMatrix::MakeFromX(DirToAxis).ToQuat();
				CameraLocationNew = RotToAxis.RotateVector(CameraLocationNew) + FVector(0.f, 0.f, PlayerPawnLocationTransformed.Z);

				if (!CameraVolumeCurrent->bCameraLocationRelativeToVolume)
				{
					CameraLocationNew += FVector(PlayerPawnLocationTransformed.X, PlayerPawnLocationTransformed.Y, 0.f);
				}

				CameraFocalPointNew = CameraVolumeCurrent->bOverrideCameraRotation
					? RotToAxis.RotateVector(CameraVolumeCurrent->CameraFocalPoint)
					: RotToAxis.RotateVector(CameraComponent->DefaultCameraFocalPoint);

				const float NewCameraRoll = CameraVolumeCurrent->bOverrideCameraRotation
					? CameraVolumeCurrent->CameraRoll
					: CameraComponent->DefaultCameraRoll;
				
				if (CameraVolumeCurrent->bFocalPointIsPlayer)
				{
					CameraFocalPointNew += PlayerPawnLocationTransformed;
					CameraRotationNew = UCameraVolumesFunctionLibrary::CalculateCameraRotation(CameraLocationNew, CameraFocalPointNew, NewCameraRoll);
				}
				else
				{
					CameraRotationNew = CameraVolumeCurrent->bOverrideCameraRotation
						? RotToAxis * CameraVolumeCurrent->CameraRotation
						// Focal point will be relative to volume, not character!
						: UCameraVolumesFunctionLibrary::CalculateCameraRotation(CameraLocationNew, CameraFocalPointNew, NewCameraRoll);
				}
			}
			else
			{
				CameraLocationNew = PlayerPawnLocationTransformed;
				CameraFocalPointNew = PlayerPawnLocationTransformed;

				CameraLocationNew += CameraVolumeCurrent->bOverrideCameraLocation
					? CameraVolumeCurrent->CameraLocation
					: CameraComponent->DefaultCameraLocation;

				CameraFocalPointNew += CameraVolumeCurrent->bOverrideCameraRotation
					? CameraVolumeCurrent->CameraFocalPoint
					: CameraComponent->DefaultCameraFocalPoint;

				if (CameraVolumeCurrent->bOverrideCameraRotation)
				{
					CameraRotationNew = CameraVolumeCurrent->CameraRotation;
				}

				if (CameraVolumeCurrent->bCameraLocationRelativeToVolume)
				{
					CameraLocationNew.Y = CameraVolumeCurrent->bOverrideCameraLocation
						? CameraVolumeCurrent->CameraLocation.Y
						: CameraComponent->DefaultCameraLocation.Y;

					CameraFocalPointNew.Y = CameraVolumeCurrent->bOverrideCameraRotation
						? CameraVolumeCurrent->CameraFocalPoint.Y
						: CameraComponent->DefaultCameraFocalPoint.Y;
				}
			}

			// Calculate camera blocking like it oriented to volume Front side
			bBlockingCalculations = bPerformBlockingCalculations
				? CameraVolumeCurrent->bPerformCameraBlocking
				: bPerformBlockingCalculations;

			if (bBlockingCalculations)
			{
				FVector NewCamVolExtentCorrected = CameraVolumeCurrent->GetCamVolExtentCorrected();
				FVector NewCamVolMinCorrected = CameraVolumeCurrent->GetCamVolMinCorrected();
				FVector NewCamVolMaxCorrected = CameraVolumeCurrent->GetCamVolMaxCorrected();

				// Calculate delta volume extent with max +Y volume coordinate
				if (!CameraVolumeCurrent->bUseZeroDepthExtent || !bIsCameraOrthographic)
				{
					FVector DeltaExtent;
					DeltaExtent.X = FMath::Abs((NewCamVolMaxCorrected.Y) * CamFOVTangens);
					DeltaExtent = FVector(DeltaExtent.X, 0.f, DeltaExtent.X / ScreenAspectRatio);
					NewCamVolExtentCorrected += DeltaExtent;
					NewCamVolMinCorrected -= DeltaExtent;
					NewCamVolMaxCorrected += DeltaExtent;
				}

				// Camera offset is always relative to camera volume local Y axis
				float CameraOffset = CameraVolumeCurrent->bUseCameraRotationAxis
					? CameraLocationNew.Size2D()
					: CameraLocationNew.Y;

				// Calculate new camera offset and screen world extent at depth of CameraOffset
				FVector ScreenExtent;

				if (bIsCameraOrthographic)
				{
					CameraFOVOWNew = CameraVolumeCurrent->GetCamVolAspectRatio() >= ScreenAspectRatio
						? FMath::Clamp(CameraFOVOWNew, CameraFOVOWNew, 2.f * NewCamVolExtentCorrected.Z * ScreenAspectRatio)
						: FMath::Clamp(CameraFOVOWNew, CameraFOVOWNew, 2.f * NewCamVolExtentCorrected.X);

					ScreenExtent.X = CameraFOVOWNew * 0.5f;
				}
				else
				{
					if (CameraVolumeCurrent->bUseCameraRotationAxis)
					{
						CameraOffset = FMath::Clamp(
							CameraOffset,
							CameraOffset,
							NewCamVolExtentCorrected.Z * ScreenAspectRatio / CamFOVTangens);
					}
					else
					{
						CameraOffset = CameraVolumeCurrent->GetCamVolAspectRatio() >= ScreenAspectRatio
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
				CameraLocationNew = CameraVolumeCurrent->bUseCameraRotationAxis
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
		CameraLocationNew = CameraVolumeCurrent->GetActorTransform().TransformPositionNoScale(CameraLocationNew);
		CameraFocalPointNew = CameraVolumeCurrent->GetActorTransform().TransformPositionNoScale(CameraFocalPointNew);
		CameraRotationNew = CameraVolumeCurrent->GetActorTransform().TransformRotation(CameraRotationNew);
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

		if (CameraComponent->bEnableCameraLocationLag)
		{
			CameraLocationNew = FMath::VInterpTo(PlayerPawnLocationOld, PlayerPawnLocation, DeltaTime, CameraComponent->CameraLocationLagSpeed);
		}

		// Final world-space values
		CameraFocalPointNew = PlayerPawnLocation + PlayerPawn->GetActorQuat().RotateVector(CameraComponent->DefaultCameraFocalPoint);
		CameraLocationNew = CameraFocalPointNew + CameraRotationNew.RotateVector(CameraComponent->DefaultCameraLocation - CameraComponent->DefaultCameraFocalPoint);
		
		bUsePlayerPawnControlRotation = true;
	}

	// Dead zone
	if (bUseDeadZone && !bFirstPass && !bUsePlayerPawnControlRotation)
	{
		ProcessDeadZone();
	}

	// Transitions and interpolations
	if (!bFirstPass)
	{
		CalculateTransitions(DeltaTime);
	}

	CameraLocationFinalNew = CameraLocationNew;
	CameraRotationFinalNew = CameraRotationNew;
	CameraFOVOWFinalNew = CameraFOVOWNew;

	// Additional camera params
	if (CameraComponent->bUseAdditionalCameraParams && !bFirstPass)
	{
		CameraLocationFinalNew += CameraComponent->AdditionalCameraLocation;
		CameraRotationFinalNew *= CameraComponent->AdditionalCameraRotation.Quaternion();
		CameraFOVOWFinalNew += bIsCameraOrthographic
			? CameraComponent->AdditionalCameraOrthoWidth
			: CameraComponent->AdditionalCameraFOV;
	}

	bFirstPass = false;

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
	BroadcastCameraVolume = CameraVolume;
	BroadcastSideInfo = SideInfo;
}

void ACameraVolumesCameraManager::CalculateTransitions(float DeltaTime)
{
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
		if (CameraComponent->bEnableCameraLocationLag && !bUsePlayerPawnControlRotation
			&& !bUseDeadZone)
		{
			CameraLocationNew = FMath::VInterpTo(CameraLocationOld, CameraLocationNew, DeltaTime, CameraComponent->CameraLocationLagSpeed);
		}

		if (CameraComponent->bEnableCameraRotationLag && !bUsePlayerPawnControlRotation
			&& !bUseDeadZone)
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
}

bool ACameraVolumesCameraManager::IsInDeadZone(FVector& InWorldLocation, FDeadZoneTransform& InDeadZoneTransform)
{
	if (APlayerController* PC = GetOwningPlayerController())
	{
		if (GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport)
		{
			FVector2D ScreenLocation;
			PC->ProjectWorldLocationToScreen(InWorldLocation, ScreenLocation);
			const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
			const FVector2D DeadZoneScreenExtent = ViewportSize * InDeadZoneTransform.DeadZoneExtent / 200.f;
			const FVector2D DeadZoneScreenOffset = ViewportSize * InDeadZoneTransform.DeadZoneOffset / 100.f;
			const FVector2D DeadZoneScreenCenter = ViewportSize / 2.f + DeadZoneScreenOffset;
			const FVector2D DeadZoneScreenMin = DeadZoneScreenCenter - DeadZoneScreenExtent;
			const FVector2D DeadZoneScreenMax = DeadZoneScreenCenter + DeadZoneScreenExtent;

			if (InDeadZoneTransform.DeadZoneRoll != 0)
			{
				FVector2D RotatedLocation = ScreenLocation - DeadZoneScreenCenter;
				RotatedLocation = RotatedLocation.GetRotated(InDeadZoneTransform.DeadZoneRoll);
				ScreenLocation = RotatedLocation + DeadZoneScreenCenter;
			}
			
			return ScreenLocation >= DeadZoneScreenMin && ScreenLocation <= DeadZoneScreenMax;
		}
	}

	return false;
}

// work in progress...
void ACameraVolumesCameraManager::ProcessDeadZone()
{
	FDeadZoneTransform DeadZoneTransform(DeadZoneExtent, DeadZoneOffset, CameraRotationNew.Rotator().Roll);
	bIsInDeadZone = IsInDeadZone(PlayerPawnLocation, DeadZoneTransform);
	if (bIsInDeadZone)
	{
		if (!(bNeedsSmoothTransition || bNeedsCutTransition))
		{
			CameraLocationNew = CameraLocationOld;
			CameraRotationNew = CameraRotationOld;
		}
	}
	else
	{
		//if (CameraComponent->bEnableCameraLocationLag)
		//{
		//	CameraLocationNew = CameraLocationOld;
		//	CameraRotationNew = CameraRotationOld;
		//}

		// should recalculate if dead zone becomes smaller
		// should recalculate if camera is relative to volume
		////if (CameraVolumeCurrent)
		////{
		////	if (CameraVolumeCurrent->bCameraLocationRelativeToVolume)
		////	{
		////		CameraLocationNew = CameraLocationOld + (PlayerPawnLocation - PlayerPawnLocationOld);
		////	}
		////	else
		////	{
		////		CameraLocationNew = CameraLocationOld + (PlayerPawnLocation - PlayerPawnLocationOld);
		////	}
		////}
		////else
		{
			CameraLocationNew = CameraLocationOld + (PlayerPawnLocation - PlayerPawnLocationOld);
		}
	}

	
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
