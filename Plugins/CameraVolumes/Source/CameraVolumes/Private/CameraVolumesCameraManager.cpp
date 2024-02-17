// redbox, 2024

#include "CameraVolumesCameraManager.h"
#include "CameraVolumesCharacterInterface.h"
#include "CameraVolumeDynamicActor.h"
#include "CameraVolumesFunctionLibrary.h"
#include "GameFramework/PlayerController.h"

ACameraVolumesCameraManager::ACameraVolumesCameraManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUpdateCamera = true;
	bProcessCameraVolumes = true;
	bPerformBlockingCalculations = true;
	CameraFOVOWOld = 90.f;
	CameraFOVOWNew = 90.f;
	bFirstPass = true;

	LoadConfig();
}

void ACameraVolumesCameraManager::SetUpdateCamera(bool bShouldUpdateCamera)
{
	bUpdateCamera = bShouldUpdateCamera;
}

void ACameraVolumesCameraManager::SetProcessCameraVolumes(bool bShouldProcess)
{
	if (bProcessCameraVolumes != bShouldProcess)
	{
		bProcessCameraVolumes = bShouldProcess;
		if (!bProcessCameraVolumes)
		{
			APawn* OwningPawn = GetOwningPlayerController()->GetPawn();
			if (OwningPawn)
			{
				ICameraVolumesCharacterInterface* PlayerCharacter = Cast<ICameraVolumesCharacterInterface>(PlayerPawn);
				if (PlayerCharacter)
				{
					UCameraVolumesCameraComponent* CamComp = PlayerCharacter->GetCameraComponent();
					if (CamComp)
					{
						CamComp->OverlappingCameraVolumes.Empty();
					}
				}
			}
		}
	}
}

void ACameraVolumesCameraManager::SetPerformBlockingCalculations(bool bShouldPerformBlockingCalculations)
{
	bPerformBlockingCalculations = bShouldPerformBlockingCalculations;
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
				// Store previous values and prepare params
				CameraVolumePrevious = CameraVolumeCurrent;
				CameraVolumeCurrent = nullptr;

				CameraComponent = PlayerCharacter->GetCameraComponent();
				if (CameraComponent)
				{
					PlayerPawnLocationOld = PlayerPawnLocation;
					PlayerPawnLocation = PlayerPawn->GetActorLocation();

					if (bProcessCameraVolumes)
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
										CameraComponent->OverlappingCameraVolumes.Emplace(CameraVolume);
									}
								}

								CameraVolumeCurrent = UCameraVolumesFunctionLibrary::GetCurrentCameraVolume(CameraComponent->OverlappingCameraVolumes, PlayerPawnLocation);
							}
							else
							{
								// There are no camera volumes overlapping character at this time,
								// so we can stop processing until player pawn overlap some camera volume again.
								SetProcessCameraVolumes(false);
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
								const FSideInfo& PassedSideInfoCurrent = CameraVolumeCurrent->GetNearestVolumeSideInfo(PlayerPawnLocation);

								if (CameraVolumePrevious)
								{
									const FSideInfo& PassedSideInfoPrevious = CameraVolumePrevious->GetNearestVolumeSideInfo(PlayerPawnLocation);

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
						}
						else if (CameraVolumePrevious) // Do we passed from volume to void?
						{
							// Use settings of side we've passed from
							const FSideInfo& PassedSideInfoPrevious = CameraVolumePrevious->GetNearestVolumeSideInfo(PlayerPawnLocation);
							SetTransitionBySideInfo(CameraVolumePrevious, PassedSideInfoPrevious);
						}
					}

					CalculateCameraParams(DeltaTime);

					// Update camera component
					CameraComponent->UpdateCamera(CameraViewInfoFinal);

#if WITH_EDITOR && DEAD_ZONES
					// Dead zone preview
					CameraComponent->UpdateDeadZonePreview(FDeadZoneTransform(DeadZoneExtent, DeadZoneOffset, DeadZoneRoll));
#endif

					// Update camera manager
					if (bIsOrthographic)
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
	}

	Super::UpdateCamera(DeltaTime);
}

void ACameraVolumesCameraManager::CalculateCameraParams(float DeltaTime)
{
	// Smooth transition from fixed old params or smooth transition was interrupted
	if (!bNeedsSmoothTransition || bSmoothTransitionInterrupted)
	{
		bSmoothTransitionInterrupted = false;
		CameraLocationOld = CameraLocationNew;
		CameraRotationOld = CameraRotationNew;
		CameraFOVOWOld = CameraFOVOWNew;
	}

	CameraLocationSourceOld = CameraLocationSourceNew;
	CameraLocationSourceNew = PlayerPawnLocation;

	bIsCameraStatic = false;
	bUsePlayerPawnControlRotation = false;

	CameraProjectionModeOld = CameraProjectionModeNew;
	CameraProjectionModeNew = CameraComponent->ProjectionMode;
	bIsOrthographic = CameraProjectionModeNew == ECameraProjectionMode::Orthographic;

	CameraLocationNew = CameraLocationSourceNew + CameraComponent->DefaultCameraLocation;
	CameraRotationNew = CameraComponent->GetDefaultCameraRotation();
	CameraFocalPointNew = CameraLocationSourceNew + CameraComponent->DefaultCameraFocalPoint;
	CameraFOVOWNew = bIsOrthographic
		? CameraComponent->DefaultCameraOrthoWidth
		: CameraComponent->DefaultCameraFieldOfView;

	bool bDoCollisionTest = CameraComponent->bDoCollisionTest;

#if DEAD_ZONES
	bUseDeadZone = false;
	bIsInDeadZone = false;

	if (CameraComponent->bUseDeadZone)
	{
		bUseDeadZone = true;
		DeadZoneExtent = CameraComponent->DeadZoneExtent;
		DeadZoneOffset = CameraComponent->DeadZoneOffset;
		bShouldCalculateDeadZoneRoll = CameraComponent->bShouldCalculateDeadZoneRoll;
	}
#endif

	// Active camera volume
	if (CameraVolumeCurrent)
	{
#if DEAD_ZONES
		// Dead zone
		if (CameraVolumeCurrent->bOverrideDeadZoneSettings)
		{
			bUseDeadZone = true;
			DeadZoneExtent = CameraVolumeCurrent->DeadZoneExtent;
			DeadZoneOffset = CameraVolumeCurrent->DeadZoneOffset;
		}
#endif

		bIsCameraStatic = CameraVolumeCurrent->GetIsCameraStatic();

#if DEAD_ZONES
		if (bIsCameraStatic)
		{
			// Do not process dead zone for fully static camera
			bUseDeadZone = false;
		}

		if (bUseDeadZone && !bFirstPass)
		{
			CalculateDeadZone();

			if (bIsInDeadZone)
			{
				if (!(bNeedsSmoothTransition || bNeedsCutTransition))
				{
					CameraLocationSourceNew = CameraLocationSourceOld;
				}
				else if (bNeedsSmoothTransition)
				{
					bSmoothTransitionInDeadZone = true;
				}
			}
			else
			{
				CameraLocationSourceNew = CameraLocationSourceOld + PlayerPawnLocation - PlayerPawnLocationOld;
				bSmoothTransitionInDeadZone = false;
			}

			CameraLocationNew = CameraLocationSourceNew + CameraComponent->DefaultCameraLocation;
			CameraFocalPointNew = CameraLocationSourceNew + CameraComponent->DefaultCameraFocalPoint;
		}
#endif

		// Camera projection
		if (bIsOrthographic)
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

		// Location and Rotation
		const FVector CameraLocationSourceTransformed = CameraVolumeCurrent->GetActorTransform().InverseTransformPositionNoScale(CameraLocationSourceNew);

		if (bIsCameraStatic)
		{
			CameraLocationNew = CameraVolumeCurrent->CameraLocation;

			if (CameraVolumeCurrent->bFocalPointIsPlayer)
			{
				CameraFocalPointNew = CameraLocationSourceTransformed + CameraVolumeCurrent->CameraFocalPoint;
				CameraRotationNew = UCameraVolumesFunctionLibrary::CalculateCameraRotation(CameraVolumeCurrent->CameraLocation, CameraFocalPointNew, CameraVolumeCurrent->CameraRoll);
			}
			else
			{
				CameraRotationNew = CameraVolumeCurrent->GetCameraRotation();
			}
		}
		else
		{
			if (CameraVolumeCurrent->bUseCameraRotationAxis)
			{
				CameraLocationNew = CameraVolumeCurrent->bOverrideCameraLocation
					? CameraVolumeCurrent->CameraLocation
					: CameraComponent->DefaultCameraLocation;

				const FVector DirToAxis = CameraLocationSourceTransformed.GetSafeNormal2D();
				const FQuat RotToAxis = FRotationMatrix::MakeFromX(DirToAxis).ToQuat();
				const FVector VerticalOffset = FVector(0.f, 0.f, CameraLocationSourceTransformed.Z);

				CameraLocationNew = RotToAxis.RotateVector(CameraLocationNew) + VerticalOffset;

				if (!CameraVolumeCurrent->bCameraLocationRelativeToVolume)
				{
					CameraLocationNew += FVector(CameraLocationSourceTransformed.X, CameraLocationSourceTransformed.Y, 0.f);
				}

				const float NewCameraRoll = CameraVolumeCurrent->bOverrideCameraRotation
					? CameraVolumeCurrent->CameraRoll
					: CameraComponent->DefaultCameraRoll;

				CameraFocalPointNew = CameraVolumeCurrent->bOverrideCameraRotation
					? RotToAxis.RotateVector(CameraVolumeCurrent->CameraFocalPoint)
					: RotToAxis.RotateVector(CameraComponent->DefaultCameraFocalPoint);

				if (CameraVolumeCurrent->bFocalPointIsPlayer)
				{
					CameraFocalPointNew += CameraLocationSourceTransformed;
					CameraRotationNew = UCameraVolumesFunctionLibrary::CalculateCameraRotation(CameraLocationNew, CameraFocalPointNew, NewCameraRoll);
				}
				else
				{
					CameraFocalPointNew += VerticalOffset;
					CameraRotationNew = CameraVolumeCurrent->bOverrideCameraRotation
						? RotToAxis * CameraVolumeCurrent->GetCameraRotation()
						// Focal point will be relative to volume, not character!
						: UCameraVolumesFunctionLibrary::CalculateCameraRotation(CameraLocationNew, CameraFocalPointNew, NewCameraRoll);
				}
			}
			else
			{
				CameraLocationNew = CameraLocationSourceTransformed;
				CameraFocalPointNew = CameraLocationSourceTransformed;

				CameraLocationNew += CameraVolumeCurrent->bOverrideCameraLocation
					? CameraVolumeCurrent->CameraLocation
					: CameraComponent->DefaultCameraLocation;

				CameraFocalPointNew += CameraVolumeCurrent->bOverrideCameraRotation
					? CameraVolumeCurrent->CameraFocalPoint
					: CameraComponent->DefaultCameraFocalPoint;

				if (CameraVolumeCurrent->bOverrideCameraRotation)
				{
					CameraRotationNew = CameraVolumeCurrent->GetCameraRotation();
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

			bDoCollisionTest = bDoCollisionTest || CameraVolumeCurrent->bDoCollisionTest;

			// Calculate camera blocking like it oriented to volume Front side
			bBlockingCalculations = bPerformBlockingCalculations
				? CameraVolumeCurrent->bPerformCameraBlocking
				: bPerformBlockingCalculations;

			if (bBlockingCalculations)
			{
				// Get screen (or at least player camera) aspect ratio for further calculations
				const float CamFOVTangens = FMath::Tan(FMath::DegreesToRadians(CameraFOVOWNew * 0.5f));
				float ScreenAspectRatio = CameraComponent->AspectRatio;

				if (APlayerController* PC = GetOwningPlayerController())
				{
					if (ULocalPlayer* LP = PC->GetLocalPlayer())
					{
						if (LP->ViewportClient)
						{
							FVector2D ViewportSize;
							LP->ViewportClient->GetViewportSize(ViewportSize);
							if (ViewportSize.Y != 0)
							{
								ScreenAspectRatio = static_cast<float>(ViewportSize.X / ViewportSize.Y);
							}
						}
					}
				}

				FVector NewCamVolExtentCorrected = CameraVolumeCurrent->GetCamVolExtentCorrected();
				FVector NewCamVolMinCorrected = CameraVolumeCurrent->GetCamVolMinCorrected();
				FVector NewCamVolMaxCorrected = CameraVolumeCurrent->GetCamVolMaxCorrected();

				// Calculate delta volume extent with max +Y volume coordinate
				if (!CameraVolumeCurrent->bUseZeroDepthExtent || !bIsOrthographic)
				{
					FVector DeltaExtent(FVector::ZeroVector);
					DeltaExtent.X = FMath::Abs((NewCamVolMaxCorrected.Y) * CamFOVTangens);
					DeltaExtent.Z = DeltaExtent.X / ScreenAspectRatio;
					NewCamVolExtentCorrected += DeltaExtent;
					NewCamVolMinCorrected -= DeltaExtent;
					NewCamVolMaxCorrected += DeltaExtent;
				}

				// Camera offset is always relative to camera volume local Y axis
				CameraOffset = CameraVolumeCurrent->bUseCameraRotationAxis
					? CameraLocationNew.Size2D()
					: CameraLocationNew.Y;

				// Calculate new camera offset and screen world extent at depth of CameraOffset
				FVector ScreenExtent(FVector::ZeroVector);

				if (bIsOrthographic)
				{
					CameraFOVOWNew = CameraVolumeCurrent->GetCamVolAspectRatio() >= ScreenAspectRatio
						? FMath::Min(
							CameraFOVOWNew,
							2.f * NewCamVolExtentCorrected.Z * ScreenAspectRatio)
						: FMath::Min(
							CameraFOVOWNew,
							2.f * NewCamVolExtentCorrected.X);

					ScreenExtent.X = CameraFOVOWNew * 0.5f;
				}
				else
				{
					if (CameraVolumeCurrent->bUseCameraRotationAxis)
					{
						CameraOffset = FMath::Min(
							CameraOffset,
							NewCamVolExtentCorrected.Z * ScreenAspectRatio / CamFOVTangens);
					}
					else
					{
						CameraOffset = CameraVolumeCurrent->GetCamVolAspectRatio() >= ScreenAspectRatio
							? FMath::Min(
								CameraOffset,
								NewCamVolExtentCorrected.Z * ScreenAspectRatio / CamFOVTangens)
							: FMath::Min(
								CameraOffset,
								NewCamVolExtentCorrected.X / CamFOVTangens);
					}

					ScreenExtent.X = FMath::Abs(CameraOffset * CamFOVTangens);
				}

				ScreenExtent.Z = ScreenExtent.X / ScreenAspectRatio;
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

		// World-space values
		CameraLocationNew = CameraVolumeCurrent->GetActorTransform().TransformPositionNoScale(CameraLocationNew);
		CameraFocalPointNew = CameraVolumeCurrent->GetActorTransform().TransformPositionNoScale(CameraFocalPointNew);
		CameraRotationNew = CameraVolumeCurrent->GetActorTransform().TransformRotation(CameraRotationNew);
	}
	else
	{
		if (CameraComponent->bUsePawnControlRotationCV)
		{
			// Player-controlled camera
			bUsePlayerPawnControlRotation = true;
#if DEAD_ZONES
			// Do not process dead zone for camera that uses control rotation
			bUseDeadZone = false;
#endif
			const FRotator CamRot = CameraComponent->GetDefaultCameraRotation().Rotator();
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
				CameraLocationNew = FMath::VInterpTo(CameraLocationSourceOld, CameraLocationSourceNew, DeltaTime, CameraComponent->CameraLocationLagSpeed);
			}

			// Final world-space values
			CameraFocalPointNew = CameraLocationSourceNew + PlayerPawn->GetActorQuat().RotateVector(CameraComponent->DefaultCameraFocalPoint);
			CameraLocationNew = CameraFocalPointNew + CameraRotationNew.RotateVector(CameraComponent->DefaultCameraLocation - CameraComponent->DefaultCameraFocalPoint);
		}
#if DEAD_ZONES
		if (bUseDeadZone && !bFirstPass)
		{
			CalculateDeadZone();
			if (bIsInDeadZone)
			{
				if (!(bNeedsSmoothTransition || bNeedsCutTransition))
				{
					CameraLocationSourceNew = CameraLocationSourceOld;
				}
				else if (bNeedsSmoothTransition)
				{
					bSmoothTransitionInDeadZone = true;
				}
			}
			else
			{
				CameraLocationSourceNew = CameraLocationSourceOld + PlayerPawnLocation - PlayerPawnLocationOld;
				bSmoothTransitionInDeadZone = false;
			}

			CameraLocationNew = CameraLocationSourceNew + CameraComponent->DefaultCameraLocation;
			CameraFocalPointNew = CameraLocationSourceNew + CameraComponent->DefaultCameraFocalPoint;
		}
#endif
	}

	// Transitions and interpolations
	if (!bFirstPass)
	{
		CalculateTransitions(DeltaTime);
	}

	CameraLocationFinalNew = CameraLocationNew;
	CameraRotationFinalNew = CameraRotationNew;
	CameraFOVOWFinalNew = CameraFOVOWNew;

	// Camera collision
	if (!bIsCameraStatic && bDoCollisionTest)
	{
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SpringArm), false, CameraComponent->GetOwner());
		FHitResult HitResult;
		GetWorld()->SweepSingleByChannel(HitResult, CameraFocalPointNew, CameraLocationNew, FQuat::Identity, CameraComponent->ProbeChannel, FCollisionShape::MakeSphere(CameraComponent->ProbeSize), QueryParams);

		if (HitResult.bBlockingHit)
		{
			CameraLocationFinalNew = HitResult.Location;
		}
	}

	// Additional camera params
	if (CameraComponent->bUseAdditionalCameraParams && !bFirstPass)
	{
		CameraLocationFinalNew += CameraComponent->AdditionalCameraLocation;
		CameraRotationFinalNew *= CameraComponent->AdditionalCameraRotation.Quaternion();
		CameraFOVOWFinalNew += bIsOrthographic
			? CameraComponent->AdditionalCameraOrthoWidth
			: CameraComponent->AdditionalCameraFOV;
	}

	CameraViewInfoFinal.Location = CameraLocationFinalNew;
	CameraViewInfoFinal.Rotation = CameraRotationFinalNew.Rotator();
	CameraViewInfoFinal.OrthoWidth = CameraFOVOWNew;
	CameraViewInfoFinal.FOV = CameraFOVOWNew;

	// Update control rotation from camera rotation
	if (!bUsePlayerPawnControlRotation && CameraComponent->bUpdateControlRotationFromCameraRotation)
	{
		if (APlayerController* PlayerController = GetOwningPlayerController())
		{
			PlayerController->SetControlRotation(CameraRotationFinalNew.Rotator());
		}
	}

	bFirstPass = false;

	// Broadcast volume changed event
	if (bBroadcastOnCameraVolumeChanged)
	{
		OnCameraVolumeChanged.Broadcast(BroadcastCameraVolume, BroadcastSideInfo);
		bBroadcastOnCameraVolumeChanged = false;
	}
}

void ACameraVolumesCameraManager::SetTransitionBySideInfo(ACameraVolumeActor* CameraVolume, const FSideInfo& SideInfo)
{
	if (!CameraVolume)
	{
		return;
	}

	// Interrupt previous smooth transition
	if (bNeedsSmoothTransition)
	{
		bSmoothTransitionInterrupted = true;
	}

	SmoothTransitionAlpha = 0.f;
	SmoothTransitionAlphaEase = 0.f;

	if (SideInfo.SideTransitionType == ESideTransitionType::ESTT_Smooth)
	{
		bNeedsSmoothTransition = true;
		bSmoothTransitionJustStarted = true;

		SmoothTransitionSpeed = CameraVolume->CameraSmoothTransitionSpeed;
		SmoothTransitionEasingFunc = (uint8)CameraVolume->SmoothTransitionEasingFunc;
		EasingFuncBlendExp = CameraVolume->EasingFuncBlendExp;
		EasingFuncSteps = CameraVolume->EasingFuncSteps;
	}
	else if (SideInfo.SideTransitionType == ESideTransitionType::ESTT_Cut)
	{
		bNeedsSmoothTransition = false;
		bNeedsCutTransition = true;
	}
	else
	{
		bNeedsSmoothTransition = false;
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
		if (bSmoothTransitionJustStarted)
		{
			CameraLocationNewFixed = CameraLocationNew;
			bSmoothTransitionJustStarted = false;
		}

#if DEAD_ZONES
		if (bSmoothTransitionInDeadZone)
		{
			CameraLocationNew = CameraLocationNewFixed;
		}
#endif
		SmoothTransitionAlpha = FMath::Clamp(SmoothTransitionAlpha + DeltaTime * SmoothTransitionSpeed, 0.f, 1.f);
		SmoothTransitionAlphaEase = UKismetMathLibrary::Ease(0.f, 1.f, SmoothTransitionAlpha, (EEasingFunc::Type)SmoothTransitionEasingFunc, EasingFuncBlendExp, EasingFuncSteps);

		if (CameraLocationNew.Equals(CameraLocationOld, 0.1f)
			|| SmoothTransitionAlpha == 1.f)
		{
			SmoothTransitionAlpha = 0.f;
			SmoothTransitionAlphaEase = 0.f;
			bNeedsSmoothTransition = false;
		}
		else
		{
			CameraLocationNew = FMath::Lerp(CameraLocationOld, CameraLocationNew, SmoothTransitionAlphaEase);
			CameraRotationNew = FQuat::Slerp(CameraRotationOld, CameraRotationNew, SmoothTransitionAlphaEase);

			if (CameraProjectionModeOld == CameraProjectionModeNew)
			{
				CameraFOVOWNew = FMath::Lerp(CameraFOVOWOld, CameraFOVOWNew, SmoothTransitionAlphaEase);
			}
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
		// Camera location lag
		bool bUseCameraLocationLag = CameraComponent->bEnableCameraLocationLag && !bUsePlayerPawnControlRotation;
		if (CameraVolumeCurrent)
		{
			bUseCameraLocationLag = CameraVolumeCurrent->bDisableCameraLocationLag ? false : bUseCameraLocationLag;
		}

		if (bUseCameraLocationLag)
		{
			CameraLocationNew = FMath::VInterpTo(CameraLocationOld, CameraLocationNew, DeltaTime, CameraComponent->CameraLocationLagSpeed);
		}

		// Camera rotation lag
		bool bUseCameraRotationLag = CameraComponent->bEnableCameraRotationLag && !bUsePlayerPawnControlRotation;
		if (CameraVolumeCurrent)
		{
			bUseCameraRotationLag = CameraVolumeCurrent->bDisableCameraRotationLag ? false : bUseCameraRotationLag;
		}

		if (bUseCameraRotationLag)
		{
			CameraRotationNew = FMath::QInterpTo(CameraRotationOld, CameraRotationNew, DeltaTime, CameraComponent->CameraRotationLagSpeed);
		}

		// Camera FOV/OW interpolation
		if (bIsOrthographic)
		{
			if (CameraComponent->bEnableCameraOrthoWidthInterp)
			{
				if (CameraProjectionModeOld == CameraProjectionModeNew)
				{
					CameraFOVOWNew = FMath::FInterpTo(CameraFOVOWOld, CameraFOVOWNew, DeltaTime, CameraComponent->CameraOrthoWidthInterpSpeed);
				}
			}
		}
		else if (CameraComponent->bEnableCameraFOVInterp)
		{
			if (CameraProjectionModeOld == CameraProjectionModeNew)
			{
				CameraFOVOWNew = FMath::FInterpTo(CameraFOVOWOld, CameraFOVOWNew, DeltaTime, CameraComponent->CameraFOVInterpSpeed);
			}
		}
	}
}

#if DEAD_ZONES
void ACameraVolumesCameraManager::CalculateDeadZone()
{
	// Automatically calculate dead zone roll depending on camera/volume rotation
	DeadZoneRoll = 0.f;
	if (bShouldCalculateDeadZoneRoll)
	{
		DeadZoneRoll = -CameraRotationNew.Rotator().Roll;
		if (CameraVolumeCurrent)
		{
			DeadZoneRoll -= CameraVolumeCurrent->GetActorRotation().Pitch;
		}
	}

	bIsInDeadZone = false;

	if (APlayerController* OwningPlayerController = GetOwningPlayerController())
	{
		FVector2D PlayerScreenLocation;
		OwningPlayerController->ProjectWorldLocationToScreen(PlayerPawnLocation, PlayerScreenLocation);

		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (LP->ViewportClient)
			{
				FVector2D ViewportSize;
				LP->ViewportClient->GetViewportSize(ViewportSize);

				const FVector2D DeadZoneScreenExtent = ViewportSize * DeadZoneExtent / 200.f;
				const FVector2D DeadZoneScreenOffset = ViewportSize * FVector2D(DeadZoneOffset.X, -DeadZoneOffset.Y) / 100.f;
				const FVector2D DeadZoneScreenCenter = ViewportSize / 2.f + DeadZoneScreenOffset;
				const FVector2D DeadZoneScreenMin = DeadZoneScreenCenter - DeadZoneScreenExtent;
				const FVector2D DeadZoneScreenMax = DeadZoneScreenCenter + DeadZoneScreenExtent;

				if (DeadZoneRoll != 0)
				{
					FVector2D RotatedLocation = PlayerScreenLocation - DeadZoneScreenCenter;
					RotatedLocation = RotatedLocation.GetRotated(DeadZoneRoll);
					PlayerScreenLocation = RotatedLocation + DeadZoneScreenCenter;
				}

				bIsInDeadZone = PlayerScreenLocation >= DeadZoneScreenMin && PlayerScreenLocation <= DeadZoneScreenMax;
			}
		}

		/** Calculate dead zone edge intersection point (E)
		 *
		 *	 _________Y_________XY
		 *	|         |         |
		 *	|         |         |
		 *	|         |         |
		 *	|         C---------X
		 *	|                   |
		 *	|                   E
		 *	|___________________|     P
		 *
		 */
		if (!bIsInDeadZone)
		{
			const FVector2D XY_C = DeadZoneScreenExtent;
			const FVector2D XAxis = FVector2D(1.f, 0.f);
			const FVector2D P_C = PlayerScreenLocation - DeadZoneScreenCenter;
			const FVector2D PCPos = P_C * P_C.GetSignVector();
			const float Cos_P_C_X = PCPos.GetSafeNormal() | XAxis;
			const float Cos_XY_C_X = XY_C.GetSafeNormal() | XAxis;
			float EC = 0.f;

			// Cos(0) = 1 ... Cos(90) = 0
			if (Cos_P_C_X >= Cos_XY_C_X)
			{
				EC = (XY_C.X / Cos_P_C_X) / PCPos.Size();
			}
			else
			{
				const FVector2D YAxis = FVector2D(0.f, 1.f);
				const float Cos_P_C_Y = PCPos.GetSafeNormal() | YAxis;
				EC = (XY_C.Y / Cos_P_C_Y) / PCPos.Size();
			}

			const FVector DeadZoneWorldCenter = CameraLocationOld + CameraRotationOld.GetForwardVector() * CameraOffset;
			DeadZoneEdgePoint = DeadZoneWorldCenter + (PlayerPawnLocation - DeadZoneWorldCenter) * EC;
		}
	}
}
#endif

FVector2D ACameraVolumesCameraManager::CalculateScreenWorldExtentAtDepth(float Depth)
{
	FVector2D ScreenExtentResult = FVector2D::ZeroVector;

	if (APlayerController* PC = GetOwningPlayerController())
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (LP->ViewportClient)
			{
				float ScreenAspectRatio = 1.f;
				FVector2D ViewportSize;
				LP->ViewportClient->GetViewportSize(ViewportSize);
				if (ViewportSize.Y != 0)
				{
					ScreenAspectRatio = static_cast<float>(ViewportSize.X / ViewportSize.Y);
				}

				ScreenExtentResult.X = bIsOrthographic
					? DefaultOrthoWidth * 0.5f
					: ScreenExtentResult.X = FMath::Abs(Depth * FMath::Tan(FMath::DegreesToRadians(DefaultFOV * 0.5f)));

				ScreenExtentResult = FVector2D(ScreenExtentResult.X, ScreenExtentResult.X / ScreenAspectRatio);
			}
		}
	}

	return ScreenExtentResult;
}
