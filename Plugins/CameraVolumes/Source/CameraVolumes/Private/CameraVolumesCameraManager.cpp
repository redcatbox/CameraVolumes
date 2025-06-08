// redbox, 2025

#include "CameraVolumesCameraManager.h"
#include "CameraVolumesCharacterInterface.h"
#include "CameraVolumeDynamicActor.h"
#include "CameraVolumesFunctionLibrary.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CameraVolumesCameraManager)

ACameraVolumesCameraManager::ACameraVolumesCameraManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUpdateCamera = true;
	bProcessCameraVolumes = true;
	bPerformBlockingCalculations = true;
	bFirstPass = true;

	LoadConfig();
}

void ACameraVolumesCameraManager::UpdateCamera(float DeltaTime)
{
	if (bUpdateCamera)
	{
		PlayerPawn = GetOwningPlayerController()->GetPawn();
		if (IsValid(PlayerPawn))
		{
			ICameraVolumesCharacterInterface* PlayerCharacter = Cast<ICameraVolumesCharacterInterface>(PlayerPawn);
			if (PlayerCharacter)
			{
				// Store previous values and prepare params
				CameraVolumePrevious = CameraVolumeCurrent;
				CameraVolumeCurrent = nullptr;

				CameraComponent = PlayerCharacter->GetCameraComponent();
				if (IsValid(CameraComponent))
				{
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
									ACameraVolumeActor* CameraVolume = Cast<ACameraVolumeActor>(OverlappingActor);
									if (IsValid(CameraVolume))
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

						if (IsValid(CameraVolumeCurrent))
						{
							if (CameraVolumeCurrent != CameraVolumePrevious) // Do we changed to another volume?
							{
								const FSideInfo& PassedSideInfoCurrent = CameraVolumeCurrent->GetNearestVolumeSideInfo(PlayerPawnLocation);

								if (IsValid(CameraVolumePrevious))
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
						else if (IsValid(CameraVolumePrevious)) // Do we passed from volume to void?
						{
							// Use settings of side we've passed from
							const FSideInfo& PassedSideInfoPrevious = CameraVolumePrevious->GetNearestVolumeSideInfo(PlayerPawnLocation);
							SetTransitionBySideInfo(CameraVolumePrevious, PassedSideInfoPrevious);
						}
					}

					CalculateCameraParams(DeltaTime);

					// Update camera component
					CameraComponent->UpdateCamera(CameraViewInfoFinal);

					// Update camera manager
					DefaultFOV = CameraFOVFinal;
					DefaultOrthoWidth = CameraOrthoWidthFinal;
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
		CameraQuatOld = CameraQuatNew;
		CameraFOVOld = CameraFOVNew;
		CameraOrthoWidthOld = CameraOrthoWidthNew;
	}

	CameraLocationSourceOld = CameraLocationSourceNew;
	CameraLocationSourceNew = PlayerPawnLocation;

	bIsCameraStatic = false;
	bUsePlayerPawnControlRotation = false;

	CameraProjectionModeOld = CameraProjectionModeNew;
	CameraProjectionModeNew = CameraComponent->ProjectionMode;
	bIsOrthographic = CameraProjectionModeNew == ECameraProjectionMode::Orthographic;

	CameraLocationNew = CameraLocationSourceNew + CameraComponent->DefaultCameraLocation;
	CameraQuatNew = CameraComponent->GetDefaultCameraRotation();
	CameraFocalPointNew = CameraLocationSourceNew + CameraComponent->DefaultCameraFocalPoint;
	CameraFOVNew = CameraComponent->DefaultCameraFieldOfView;
	CameraOrthoWidthNew = CameraComponent->DefaultCameraOrthoWidth;

	bool bDoCollisionTest = CameraComponent->bDoCollisionTest;

	// Active camera volume
	if (IsValid(CameraVolumeCurrent))
	{
		bIsCameraStatic = CameraVolumeCurrent->GetIsCameraStatic();

		// Camera projection
		if (CameraVolumeCurrent->bOverrideCameraOrthoWidth)
		{
			CameraOrthoWidthNew = CameraVolumeCurrent->CameraOrthoWidth;
		}

		if (CameraVolumeCurrent->bOverrideCameraFieldOfView)
		{
			CameraFOVNew = CameraVolumeCurrent->CameraFieldOfView;
		}

		// Location and Rotation
		const FVector CameraLocationSourceLocal = CameraVolumeCurrent->GetActorTransform().InverseTransformPositionNoScale(CameraLocationSourceNew);

		bIsCameraStatic = CameraVolumeCurrent->GetIsCameraStatic();
		if (bIsCameraStatic)
		{
			CameraLocationNew = CameraVolumeCurrent->CameraLocation;

			if (CameraVolumeCurrent->bFocalPointIsPlayer)
			{
				CameraFocalPointNew = CameraLocationSourceLocal + CameraVolumeCurrent->CameraFocalPoint;
				CameraQuatNew = UCameraVolumesFunctionLibrary::CalculateCameraRotation(CameraVolumeCurrent->CameraLocation, CameraFocalPointNew, CameraVolumeCurrent->CameraRoll);
			}
			else
			{
				CameraQuatNew = CameraVolumeCurrent->GetCameraRotation();
			}
		}
		else
		{
			if (CameraVolumeCurrent->bUseCameraRotationAxis)
			{
				CameraLocationNew = CameraVolumeCurrent->bOverrideCameraLocation
					? CameraVolumeCurrent->CameraLocation
					: CameraComponent->DefaultCameraLocation;

				const FVector DirToAxis = CameraLocationSourceLocal.GetSafeNormal2D();
				const FQuat RotToAxis = FRotationMatrix::MakeFromX(DirToAxis).ToQuat();
				const FVector VerticalOffset = FVector(0.f, 0.f, CameraLocationSourceLocal.Z);

				CameraLocationNew = RotToAxis.RotateVector(CameraLocationNew) + VerticalOffset;

				if (!CameraVolumeCurrent->bCameraLocationRelativeToVolume)
				{
					CameraLocationNew += FVector(CameraLocationSourceLocal.X, CameraLocationSourceLocal.Y, 0.f);
				}

				const float NewCameraRoll = CameraVolumeCurrent->bOverrideCameraRotation
					? CameraVolumeCurrent->CameraRoll
					: CameraComponent->DefaultCameraRoll;

				CameraFocalPointNew = CameraVolumeCurrent->bOverrideCameraRotation
					? RotToAxis.RotateVector(CameraVolumeCurrent->CameraFocalPoint)
					: RotToAxis.RotateVector(CameraComponent->DefaultCameraFocalPoint);

				if (CameraVolumeCurrent->bFocalPointIsPlayer)
				{
					CameraFocalPointNew += CameraLocationSourceLocal;
					CameraQuatNew = UCameraVolumesFunctionLibrary::CalculateCameraRotation(CameraLocationNew, CameraFocalPointNew, NewCameraRoll);
				}
				else
				{
					CameraFocalPointNew += VerticalOffset;
					CameraQuatNew = CameraVolumeCurrent->bOverrideCameraRotation
						? RotToAxis * CameraVolumeCurrent->GetCameraRotation()
						// Focal point will be relative to volume, not character!
						: UCameraVolumesFunctionLibrary::CalculateCameraRotation(CameraLocationNew, CameraFocalPointNew, NewCameraRoll);
				}
			}
			else
			{
				CameraLocationNew = CameraLocationSourceLocal;
				CameraFocalPointNew = CameraLocationSourceLocal;

				CameraLocationNew += CameraVolumeCurrent->bOverrideCameraLocation
					? CameraVolumeCurrent->CameraLocation
					: CameraComponent->DefaultCameraLocation;

				CameraFocalPointNew += CameraVolumeCurrent->bOverrideCameraRotation
					? CameraVolumeCurrent->CameraFocalPoint
					: CameraComponent->DefaultCameraFocalPoint;

				if (CameraVolumeCurrent->bOverrideCameraRotation)
				{
					CameraQuatNew = CameraVolumeCurrent->GetCameraRotation();
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
				APlayerController* PC = GetOwningPlayerController();
				if (IsValid(PC))
				{
					ULocalPlayer* LP = PC->GetLocalPlayer();
					if (IsValid(LP))
					{
						if (IsValid(LP->ViewportClient))
						{
							FVector2D ViewportSize;
							LP->ViewportClient->GetViewportSize(ViewportSize);
							if (ViewportSize.X > 0 && ViewportSize.Y > 0)
							{
								// Aspect ratio axis constraint
								const TEnumAsByte<enum EAspectRatioAxisConstraint> AspectRatioAxisConstraint =
									CameraComponent->bOverrideAspectRatioAxisConstraint
									? CameraComponent->AspectRatioAxisConstraint
									: LP->AspectRatioAxisConstraint;

								const bool bMaintainXFOV =
									((ViewportSize.X > ViewportSize.Y) && (AspectRatioAxisConstraint == AspectRatio_MajorAxisFOV))
									|| (AspectRatioAxisConstraint == AspectRatio_MaintainXFOV)
									|| bIsOrthographic;

								float HalfFOVRad = FMath::DegreesToRadians(CameraFOVNew * 0.5f);
								float ScreenAspectRatio = static_cast<float>(ViewportSize.X / ViewportSize.Y);

								if (!bMaintainXFOV)
								{
									HalfFOVRad = FMath::Atan(FMath::Tan(HalfFOVRad) / CameraComponent->AspectRatio);
									ScreenAspectRatio = 1.f / ScreenAspectRatio;
								}

								const float HalfFOVTan = FMath::Tan(HalfFOVRad);

								FVector NewCamVolExtentCorrected = CameraVolumeCurrent->GetCamVolExtentCorrected();
								FVector NewCamVolMinCorrected = CameraVolumeCurrent->GetCamVolMinCorrected();
								FVector NewCamVolMaxCorrected = CameraVolumeCurrent->GetCamVolMaxCorrected();

								// Calculate delta volume extent with max +Y volume coordinate
								if (!bIsOrthographic && !CameraVolumeCurrent->bUseZeroDepthExtent)
								{
									FVector DeltaExtent(FVector::ZeroVector);
									
									if (bMaintainXFOV)
									{
										DeltaExtent.X = FMath::Abs(NewCamVolMaxCorrected.Y * HalfFOVTan);
										DeltaExtent.Z = DeltaExtent.X / ScreenAspectRatio;
									}
									else
									{
										DeltaExtent.Z = FMath::Abs(NewCamVolMaxCorrected.Y * HalfFOVTan);
										DeltaExtent.X = DeltaExtent.Z / ScreenAspectRatio;
									}

									NewCamVolExtentCorrected += DeltaExtent;
									NewCamVolMinCorrected -= DeltaExtent;
									NewCamVolMaxCorrected += DeltaExtent;
								}

								// Camera offset is always relative to camera volume local Y axis
								float CameraOffset = CameraVolumeCurrent->bUseCameraRotationAxis
									? CameraLocationNew.Size2D()
									: CameraLocationNew.Y;

								// Calculate new camera offset and screen world extent at depth of CameraOffset
								FVector ScreenExtent(FVector::ZeroVector);

								if (bIsOrthographic)
								{
									CameraOrthoWidthNew = CameraVolumeCurrent->GetCamVolAspectRatio() >= ScreenAspectRatio
										? FMath::Min(CameraOrthoWidthNew, 2.f * NewCamVolExtentCorrected.Z * ScreenAspectRatio)
										: FMath::Min(CameraOrthoWidthNew, 2.f * NewCamVolExtentCorrected.X);

									ScreenExtent.X = CameraOrthoWidthNew * 0.5f;
									ScreenExtent.Z = ScreenExtent.X / ScreenAspectRatio;
								}
								else
								{
									if (CameraVolumeCurrent->bUseCameraRotationAxis)
									{
										if (bMaintainXFOV)
										{
											CameraOffset = FMath::Min(
												CameraOffset,
												NewCamVolExtentCorrected.Z * ScreenAspectRatio / HalfFOVTan);
										}
										else
										{
											CameraOffset = FMath::Min(
												CameraOffset,
												NewCamVolExtentCorrected.Z / HalfFOVTan);
										}
									}
									else
									{
										if (bMaintainXFOV)
										{
											CameraOffset = CameraVolumeCurrent->GetCamVolAspectRatio() >= ScreenAspectRatio
												? FMath::Min(
													CameraOffset,
													NewCamVolExtentCorrected.Z * ScreenAspectRatio / HalfFOVTan)
												: FMath::Min(
													CameraOffset,
													NewCamVolExtentCorrected.X / HalfFOVTan);
										}
										else
										{
											CameraOffset = 1.f / CameraVolumeCurrent->GetCamVolAspectRatio() >= ScreenAspectRatio
												? FMath::Min(
													CameraOffset,
													NewCamVolExtentCorrected.X * ScreenAspectRatio / HalfFOVTan)
												: FMath::Min(
													CameraOffset,
													NewCamVolExtentCorrected.Z / HalfFOVTan);
										}
									}

									if (bMaintainXFOV)
									{
										ScreenExtent.X = FMath::Abs(CameraOffset * HalfFOVTan);
										ScreenExtent.Z = ScreenExtent.X / ScreenAspectRatio;
									}
									else
									{
										ScreenExtent.Z = FMath::Abs(CameraOffset * HalfFOVTan);
										ScreenExtent.X = ScreenExtent.Z / ScreenAspectRatio;
									}
								}

								const FVector ScreenMin = NewCamVolMinCorrected + ScreenExtent;
								const FVector ScreenMax = NewCamVolMaxCorrected - ScreenExtent;

								// Perform camera blocking only on top and bottom sides
								if (CameraVolumeCurrent->bUseCameraRotationAxis)
								{
									CameraLocationNew = FVector(
										CameraLocationNew.GetSafeNormal2D().X * CameraOffset,
										CameraLocationNew.GetSafeNormal2D().Y * CameraOffset,
										FMath::Clamp(CameraLocationNew.Z, ScreenMin.Z, ScreenMax.Z));
								}
								else
								{
									CameraLocationNew = FVector(
										FMath::Clamp(CameraLocationNew.X, ScreenMin.X, ScreenMax.X),
										CameraOffset,
										FMath::Clamp(CameraLocationNew.Z, ScreenMin.Z, ScreenMax.Z));
								}
							}
						}
					}
				}
			}
		}

		// World-space values
		CameraLocationNew = CameraVolumeCurrent->GetActorTransform().TransformPositionNoScale(CameraLocationNew);
		CameraFocalPointNew = CameraVolumeCurrent->GetActorTransform().TransformPositionNoScale(CameraFocalPointNew);
		CameraQuatNew = CameraVolumeCurrent->GetActorTransform().TransformRotation(CameraQuatNew);
	}
	else
	{
		if (CameraComponent->bUsePawnControlRotationCV)
		{
			// Player-controlled camera
			bUsePlayerPawnControlRotation = true;

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

			CameraQuatNew = PawnViewRot.Quaternion();

			if (CameraComponent->bEnableCameraRotationLag)
			{
				CameraQuatNew = FMath::QInterpTo(CameraQuatOld, CameraQuatNew, DeltaTime, CameraComponent->CameraRotationLagSpeed);
			}

			if (CameraComponent->bEnableCameraLocationLag)
			{
				CameraLocationNew = FMath::VInterpTo(CameraLocationSourceOld, CameraLocationSourceNew, DeltaTime, CameraComponent->CameraLocationLagSpeed);
			}

			// Final world-space values
			CameraFocalPointNew = CameraLocationSourceNew + PlayerPawn->GetActorQuat().RotateVector(CameraComponent->DefaultCameraFocalPoint);
			CameraLocationNew = CameraFocalPointNew + CameraQuatNew.RotateVector(CameraComponent->DefaultCameraLocation - CameraComponent->DefaultCameraFocalPoint);
		}
	}

	// Transitions and interpolations
	if (!bFirstPass)
	{
		CalculateTransitions(DeltaTime);
	}

	CameraLocationFinal = CameraLocationNew;
	CameraQuatFinal = CameraQuatNew;
	CameraFOVFinal = CameraFOVNew;
	CameraOrthoWidthFinal = CameraOrthoWidthNew;

	// Camera collision
	if (!bIsCameraStatic && bDoCollisionTest)
	{
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SpringArm), false, CameraComponent->GetOwner());
		FHitResult HitResult;
		GetWorld()->SweepSingleByChannel(HitResult, CameraFocalPointNew, CameraLocationNew, FQuat::Identity, CameraComponent->ProbeChannel, FCollisionShape::MakeSphere(CameraComponent->ProbeSize), QueryParams);

		if (HitResult.bBlockingHit)
		{
			CameraLocationFinal = HitResult.Location;
		}
	}

	// Additional camera params
	if (CameraComponent->bUseAdditionalCameraParams && !bFirstPass)
	{
		CameraLocationFinal += CameraComponent->AdditionalCameraLocation;
		CameraQuatFinal *= CameraComponent->AdditionalCameraRotation.Quaternion();
		CameraFOVFinal += CameraComponent->AdditionalCameraFOV;
		CameraOrthoWidthFinal += CameraComponent->AdditionalCameraOrthoWidth;
	}

	CameraViewInfoFinal.Location = CameraLocationFinal;
	CameraViewInfoFinal.Rotation = CameraQuatFinal.Rotator();
	CameraViewInfoFinal.FOV = CameraFOVFinal;
	CameraViewInfoFinal.OrthoWidth = CameraOrthoWidthFinal;

	// Update control rotation from camera rotation
	if (!bUsePlayerPawnControlRotation && CameraComponent->bUpdateControlRotationFromCameraRotation)
	{
		APlayerController* PC = GetOwningPlayerController();
		if (IsValid(PC))
		{
			PC->SetControlRotation(CameraQuatFinal.Rotator());
		}
	}

	bFirstPass = false;

	// Broadcast volume changed event
	if (bBroadcastOnCameraVolumeChanged)
	{
		bBroadcastOnCameraVolumeChanged = false;
		OnCameraVolumeChanged.Broadcast(BroadcastCameraVolume, BroadcastSideInfo);
	}
}

void ACameraVolumesCameraManager::SetTransitionBySideInfo(const ACameraVolumeActor* CameraVolume, const FSideInfo& SideInfo)
{
	if (!IsValid(CameraVolume))
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
		SmoothTransitionEasingFunc = static_cast<uint8>(CameraVolume->SmoothTransitionEasingFunc);
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

		SmoothTransitionAlpha = FMath::Clamp(SmoothTransitionAlpha + DeltaTime * SmoothTransitionSpeed, 0.f, 1.f);
		SmoothTransitionAlphaEase = UKismetMathLibrary::Ease(0.f, 1.f, SmoothTransitionAlpha, static_cast<EEasingFunc::Type>(SmoothTransitionEasingFunc), EasingFuncBlendExp, EasingFuncSteps);

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
			CameraQuatNew = FQuat::Slerp(CameraQuatOld, CameraQuatNew, SmoothTransitionAlphaEase);

			if (CameraProjectionModeOld == CameraProjectionModeNew)
			{
				if (bIsOrthographic)
				{
					CameraOrthoWidthNew = FMath::Lerp(CameraOrthoWidthOld, CameraOrthoWidthNew, SmoothTransitionAlphaEase);
				}
				else
				{
					CameraFOVNew = FMath::Lerp(CameraFOVOld, CameraFOVNew, SmoothTransitionAlphaEase);
				}
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
		// Camera lags
		bool bUseCameraLocationLag = CameraComponent->bEnableCameraLocationLag && !bUsePlayerPawnControlRotation;
		bool bUseCameraRotationLag = CameraComponent->bEnableCameraRotationLag && !bUsePlayerPawnControlRotation;
		if (IsValid(CameraVolumeCurrent))
		{
			bUseCameraLocationLag = CameraVolumeCurrent->bDisableCameraLocationLag ? false : bUseCameraLocationLag;
			bUseCameraRotationLag = CameraVolumeCurrent->bDisableCameraRotationLag ? false : bUseCameraRotationLag;
		}

		if (bUseCameraLocationLag)
		{
			CameraLocationNew = FMath::VInterpTo(CameraLocationOld, CameraLocationNew, DeltaTime, CameraComponent->CameraLocationLagSpeed);
		}

		if (bUseCameraRotationLag)
		{
			CameraQuatNew = FMath::QInterpTo(CameraQuatOld, CameraQuatNew, DeltaTime, CameraComponent->CameraRotationLagSpeed);
		}

		// Camera FOV/OW interpolation
		if (bIsOrthographic)
		{
			if (CameraComponent->bEnableCameraOrthoWidthInterp)
			{
				if (CameraProjectionModeOld == CameraProjectionModeNew)
				{
					CameraOrthoWidthNew = FMath::FInterpTo(CameraOrthoWidthOld, CameraOrthoWidthNew, DeltaTime, CameraComponent->CameraOrthoWidthInterpSpeed);
				}
			}
		}
		else if (CameraComponent->bEnableCameraFOVInterp)
		{
			if (CameraProjectionModeOld == CameraProjectionModeNew)
			{
				CameraFOVNew = FMath::FInterpTo(CameraFOVOld, CameraFOVNew, DeltaTime, CameraComponent->CameraFOVInterpSpeed);
			}
		}
	}
}

FVector2D ACameraVolumesCameraManager::CalculateScreenWorldExtentAtDepth(float Depth)
{
	FVector2D ScreenExtentResult = FVector2D::ZeroVector;

	APlayerController* PC = GetOwningPlayerController();
	if (IsValid(PC))
	{
		ULocalPlayer* LP = PC->GetLocalPlayer();
		if (IsValid(LP))
		{
			if (IsValid(LP->ViewportClient) && IsValid(CameraComponent))
			{
				FVector2D ViewportSize;
				LP->ViewportClient->GetViewportSize(ViewportSize);
				if (ViewportSize.X > 0 && ViewportSize.Y > 0)
				{
					const TEnumAsByte<enum EAspectRatioAxisConstraint> AspectRatioAxisConstraint =
						CameraComponent->bOverrideAspectRatioAxisConstraint
						? CameraComponent->AspectRatioAxisConstraint
						: LP->AspectRatioAxisConstraint;

					const bool bMaintainXFOV =
						((ViewportSize.X > ViewportSize.Y) && (AspectRatioAxisConstraint == AspectRatio_MajorAxisFOV))
						|| (AspectRatioAxisConstraint == AspectRatio_MaintainXFOV)
						|| bIsOrthographic;

					float HalfFOVRad = FMath::DegreesToRadians(DefaultFOV * 0.5f);
					float AxisMult = static_cast<float>(ViewportSize.X / ViewportSize.Y);

					if (bMaintainXFOV)
					{
						ScreenExtentResult.X = bIsOrthographic
							? DefaultOrthoWidth * 0.5f
							: FMath::Abs(Depth * FMath::Tan(HalfFOVRad));

						ScreenExtentResult = FVector2D(ScreenExtentResult.X, ScreenExtentResult.X / AxisMult);
					}
					else
					{
						HalfFOVRad = FMath::Atan(FMath::Tan(HalfFOVRad) / CameraComponent->AspectRatio);
						AxisMult = 1.f / AxisMult;

						ScreenExtentResult.Y = FMath::Abs(Depth * FMath::Tan(HalfFOVRad));
						ScreenExtentResult = FVector2D(ScreenExtentResult.Y / AxisMult, ScreenExtentResult.Y);
					}
				}
			}
		}
	}

	return ScreenExtentResult;
}
