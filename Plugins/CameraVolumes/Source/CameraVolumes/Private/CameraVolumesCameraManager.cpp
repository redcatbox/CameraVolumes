//Dmitriy Barannik aka redbox, 2019

#include "CameraVolumesCameraManager.h"
#include "CameraVolumesCharacterInterface.h"
#include "GameFramework/PlayerController.h"

ACameraVolumesCameraManager::ACameraVolumesCameraManager()
{
	bUpdateCamera = true;
	bCheckCameraVolumes = true;
	bPerformBlockingCalculations = true;
	bUseSimpleOverlappingCalculations = false;
	CameraVolumePrevious = nullptr;
	CameraVolumeCurrent = nullptr;
	OldCameraLocation = FVector::ZeroVector;
	NewCameraLocation = FVector::ZeroVector;
	OldCameraRotation = FQuat();
	NewCameraRotation = FQuat();
	OldCameraFOV_OW = 90.f;
	NewCameraFOV_OW = 90.f;
	bIsCameraOrthographic = false;
	bNeedsSmoothTransition = false;
	bNeedsCutTransition = false;
}

void ACameraVolumesCameraManager::SetUpdateCamera(bool bNewUpdateCamera)
{
	bUpdateCamera = bNewUpdateCamera;
}

void ACameraVolumesCameraManager::SetCheckCameraVolumes(bool bNewCheck)
{
	bCheckCameraVolumes = bNewCheck;
}

void ACameraVolumesCameraManager::SetPerformBlockingCalculations(bool bNewPerformBlockingCalculations)
{
	bPerformBlockingCalculations = bNewPerformBlockingCalculations;
}

void ACameraVolumesCameraManager::SelectPerformBlockingCalculations(bool bCameraVolumePerformCameraBlocking)
{
	if (bPerformBlockingCalculations)
		bBlockingCalculations = bCameraVolumePerformCameraBlocking;
	else
		bBlockingCalculations = bPerformBlockingCalculations;
}

void ACameraVolumesCameraManager::SetUseSimpleOverlappingCalculations(bool bNewUseSimpleOverlappingCalculations)
{
	bUseSimpleOverlappingCalculations = bNewUseSimpleOverlappingCalculations;
}

void ACameraVolumesCameraManager::UpdateCamera(float DeltaTime)
{
	if (bUpdateCamera)
	{
		APawn* PlayerPawn = GetOwningPlayerController()->GetPawn();
		if (PlayerPawn)
		{
			ICameraVolumesCharacterInterface* PlayerCharacter = Cast<ICameraVolumesCharacterInterface>(PlayerPawn);
			if (PlayerCharacter)
			{
				// Prepare params
				CameraVolumePrevious = CameraVolumeCurrent;
				CameraVolumeCurrent = nullptr;
				CameraComponent = PlayerCharacter->GetCameraComponent();
				bIsCameraOrthographic = CameraComponent->GetIsCameraOrthographic();
				PlayerPawnLocation = PlayerPawn->GetActorLocation();
				OldCameraLocation = NewCameraLocation;
				OldCameraRotation = NewCameraRotation;
				NewCameraLocation = PlayerPawnLocation + CameraComponent->DefaultCameraLocation;
				NewCameraRotation = CameraComponent->DefaultCameraRotation;
				OldCameraFOV_OW = NewCameraFOV_OW;

				if (bIsCameraOrthographic)
					NewCameraFOV_OW = CameraComponent->DefaultCameraOrthoWidth;
				else
					NewCameraFOV_OW = CameraComponent->DefaultCameraFieldOfView;

				if (bCheckCameraVolumes)
				{
					// Try to get overlapping camera volumes stored in camera component
					if (CameraComponent->OverlappingCameraVolumes.Num() > 0)
						OverlappingCameraVolumes = CameraComponent->OverlappingCameraVolumes;
					// Try to get camera volumes from actors overlapping pawn
					else
					{
						OverlappingActors.Empty();
						PlayerPawn->GetOverlappingActors(OverlappingActors, ACameraVolumeActor::StaticClass());
						if (OverlappingActors.Num() > 0)
						{
							for (AActor* Actor : OverlappingActors)
							{
								ACameraVolumeActor* CameraVolume = Cast<ACameraVolumeActor>(Actor);
								if (CameraVolume)
									OverlappingCameraVolumes.Add(CameraVolume);
							}
						}
						else
							bCheckCameraVolumes = false;
						// There are no camera volumes overlapping character at this time,
						// so we don't need this check until player pawn overlap some camera volume again.
					}

					if (OverlappingCameraVolumes.Num() > 0)
					{
						if (bUseSimpleOverlappingCalculations)
							CameraVolumeCurrent = UCameraVolumesFunctionLibrary::GetCurrentCameraVolumeSimple(OverlappingCameraVolumes);
						else
							CameraVolumeCurrent = UCameraVolumesFunctionLibrary::GetCurrentCameraVolume(OverlappingCameraVolumes, PlayerPawnLocation);
					}

					if (CameraVolumeCurrent)
					{
						if (CameraVolumeCurrent != CameraVolumePrevious) // Do we changed to another volume?
						{
							// Check for dynamic camera volumes
							ACameraVolumeDynamicActor* CameraVolumeCurrentDynamic = Cast<ACameraVolumeDynamicActor>(CameraVolumeCurrent);
							if (CameraVolumeCurrentDynamic)
								CameraVolumeCurrentDynamic->SetActive(true);
							ACameraVolumeDynamicActor* CameraVolumePreviousDynamic = Cast<ACameraVolumeDynamicActor>(CameraVolumePrevious);
							if (CameraVolumePreviousDynamic)
								CameraVolumePreviousDynamic->SetActive(false);

							ESide PassedSideCurrent;
							PassedSideCurrent = CameraVolumeCurrent->GetNearestVolumeSide(PlayerPawnLocation);

							if (CameraVolumePrevious)
							{
								ESide PassedSidePrevious;
								PassedSidePrevious = CameraVolumePrevious->GetNearestVolumeSide(PlayerPawnLocation);

								if (UCameraVolumesFunctionLibrary::CompareSidesPairs(PassedSideCurrent, PassedSidePrevious, CameraVolumePrevious->bUse6DOFVolume))
									// We've passed to nearby volume
									// Use settings of side we have passed to
									SetTransitionBySideInfo(CameraVolumeCurrent, PassedSideCurrent);
								else
								{
									// we've passed to volume with another priority
									if (CameraVolumeCurrent->Priority > CameraVolumePrevious->Priority)
										// Use settings of side we have passed to
										SetTransitionBySideInfo(CameraVolumeCurrent, PassedSideCurrent);
									else
										// Use settings of side we have passed from
										SetTransitionBySideInfo(CameraVolumePrevious, PassedSidePrevious);
								}
							}
							else
								// We've passed from void to volume
								SetTransitionBySideInfo(CameraVolumeCurrent, PassedSideCurrent);
						}

						SelectPerformBlockingCalculations(CameraVolumeCurrent->bPerformCameraBlocking);
						CalcNewCameraParams(CameraVolumeCurrent, DeltaTime);
					}
					else if (CameraVolumePrevious) // Do we passed from volume to void?
					{
						// Check for dynamic camera volumes
						ACameraVolumeDynamicActor* CameraVolumePreviousDynamic = Cast<ACameraVolumeDynamicActor>(CameraVolumePrevious);
						if (CameraVolumePreviousDynamic)
							CameraVolumePreviousDynamic->SetActive(false);

						// Use settings of side we've passed from
						ESide PassedSidePrevious = CameraVolumePrevious->GetNearestVolumeSide(PlayerPawnLocation);
						SetTransitionBySideInfo(CameraVolumePrevious, PassedSidePrevious);
						CalcNewCameraParams(nullptr, DeltaTime);
					}
					else
						CalcNewCameraParams(nullptr, DeltaTime);
				}
				else
					CalcNewCameraParams(nullptr, DeltaTime);

				CameraComponent->UpdateCamera(NewCameraLocation, NewCameraRotation, NewCameraFOV_OW);
			}
		}
	}

	Super::UpdateCamera(DeltaTime);
}

void ACameraVolumesCameraManager::CalcNewCameraParams(ACameraVolumeActor* CameraVolume, float DeltaTime)
{
	if (CameraVolume)
	{
		if (bIsCameraOrthographic)
		{
			if (CameraVolume->bOverrideCameraOrthoWidth)
				NewCameraFOV_OW = CameraVolume->CameraOrthoWidth;
		}
		else
		{
			if (CameraVolume->bOverrideCameraFieldOfView)
				NewCameraFOV_OW = CameraVolume->CameraFieldOfView;
		}

		// Get screen (or at least player camera) aspect ratio for further calculations
		float PlayerCamFOVTangens = FMath::Tan(FMath::DegreesToRadians(NewCameraFOV_OW * 0.5f));
		float ScreenAspectRatio;

		if (GEngine->GameViewport->Viewport)
		{
			const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
			ScreenAspectRatio = ViewportSize.X / ViewportSize.Y;
		}
		else
			ScreenAspectRatio = CameraComponent->AspectRatio;

		// Location and Rotation
		if (CameraVolume->GetIsCameraStatic())
		{
			NewCameraLocation = CameraVolume->GetActorLocation() + CameraVolume->CameraLocation;

			if (CameraVolume->bFocalPointIsPlayer)
				NewCameraRotation = UCameraVolumesFunctionLibrary::CalculateCameraRotationToCharacter(CameraVolume->CameraLocation, CameraVolume->CameraFocalPoint, CameraVolume->CameraRoll, PlayerPawnLocation, CameraVolume->GetActorLocation());
			else
				NewCameraRotation = CameraVolume->CameraRotation;
		}
		else
		{
			if (CameraVolume->bOverrideCameraLocation)
				NewCameraLocation = PlayerPawnLocation + CameraVolume->CameraLocation;

			if (bBlockingCalculations)
			{
				// Camera offset is always relative to CameraVolume->GetActorLocation().Y
				float CameraOffset;

				if (CameraVolume->bCameraLocationRelativeToVolume)
					CameraOffset = CameraVolume->CameraLocation.Y; //Side-scroller
					//CameraOffset = CameraVolume->CameraLocation.Z; //Top-down
				else
					CameraOffset = NewCameraLocation.Y - CameraVolume->GetActorLocation().Y; //Side-scroller
					//CameraOffset = NewCameraLocation.Z - CameraVolume->GetActorLocation().Z; //Top-down

				FVector NewCamVolExtentCorrected = CameraVolume->CamVolExtentCorrected;
				FVector NewCamVolWorldMinCorrected = CameraVolume->CamVolWorldMinCorrected;
				FVector NewCamVolWorldMaxCorrected = CameraVolume->CamVolWorldMaxCorrected;
				// Calculate delta volume extent with +Y volume extent (+Z in top-down)
				if (!CameraVolume->bUseZeroDepthExtent || !bIsCameraOrthographic)
				{
					FVector DeltaExtent = FVector::ZeroVector;
					//Side-scroller
					DeltaExtent.X = FMath::Abs((NewCamVolWorldMaxCorrected.Y - CameraVolume->GetActorLocation().Y) * PlayerCamFOVTangens);
					DeltaExtent = FVector(DeltaExtent.X, 0.f, DeltaExtent.X / ScreenAspectRatio);
					//Top-down
					//DeltaExtent.X = FMath::Abs((NewCamVolWorldMaxCorrected.Z - CameraVolume->GetActorLocation().Z) * PlayerCamFOVTangens);
					//DeltaExtent = FVector(DeltaExtent.X, DeltaExtent.X / ScreenAspectRatio, 0.f);

					NewCamVolExtentCorrected += DeltaExtent;
					NewCamVolWorldMinCorrected -= DeltaExtent;
					NewCamVolWorldMaxCorrected += DeltaExtent;
				}

				// Calculate new camera offset and screen world extent at depth (CameraOffset)
				FVector ScreenExtent = FVector::ZeroVector;

				if (bIsCameraOrthographic)
				{
					if (CameraVolume->CamVolAspectRatio >= ScreenAspectRatio) // Horizontal movement
						NewCameraFOV_OW = FMath::Clamp(NewCameraFOV_OW, NewCameraFOV_OW, 2.f * NewCamVolExtentCorrected.Z * ScreenAspectRatio); //Side-scroller
						//CameraOffset = FMath::Clamp(CameraOffset, CameraOffset, 2.f * NewCamVolExtentCorrected.Y * ScreenAspectRatio); //Top-down
					else // Vertical movement
						NewCameraFOV_OW = FMath::Clamp(NewCameraFOV_OW, NewCameraFOV_OW, 2.f * NewCamVolExtentCorrected.X);

					ScreenExtent.X = NewCameraFOV_OW * 0.5f;
				}
				else
				{
					if (CameraVolume->CamVolAspectRatio >= ScreenAspectRatio) // Horizontal movement
						CameraOffset = FMath::Clamp(CameraOffset, CameraOffset, NewCamVolExtentCorrected.Z * ScreenAspectRatio / PlayerCamFOVTangens); //Side-scroller
						//CameraOffset = FMath::Clamp(CameraOffset, CameraOffset, NewCamVolExtentCorrected.Y * ScreenAspectRatio / PlayerCamFOVTangens); //Top-down
					else // Vertical movement
						CameraOffset = FMath::Clamp(CameraOffset, CameraOffset, NewCamVolExtentCorrected.X / PlayerCamFOVTangens);

					ScreenExtent.X = FMath::Abs(CameraOffset * PlayerCamFOVTangens);
				}

				ScreenExtent = FVector(ScreenExtent.X, 0.f, ScreenExtent.X / ScreenAspectRatio); //Side-scroller
				//ScreenExtent = FVector(ScreenExtent.X, ScreenExtent.X / ScreenAspectRatio, 0.f); //Top-down
				FVector ScreenWorldMin = NewCamVolWorldMinCorrected + ScreenExtent;
				FVector ScreenWorldMax = NewCamVolWorldMaxCorrected - ScreenExtent;

				// New camera location
				//Side-scroller
				NewCameraLocation = FVector(
					FMath::Clamp(NewCameraLocation.X, ScreenWorldMin.X, ScreenWorldMax.X),
					CameraOffset + CameraVolume->GetActorLocation().Y,
					FMath::Clamp(NewCameraLocation.Z, ScreenWorldMin.Z, ScreenWorldMax.Z));
				//Top-down
				//NewCameraLocation = FVector(
				//	FMath::Clamp(NewCameraLocation.X, ScreenWorldMin.X, ScreenWorldMax.X),
				//	FMath::Clamp(NewCameraLocation.Y, ScreenWorldMin.Y, ScreenWorldMax.Y),
				//	CameraOffset + CameraVolume->GetActorLocation().Z);
			}

			// New camera rotation
			if (CameraVolume->bOverrideCameraRotation)
				NewCameraRotation = UCameraVolumesFunctionLibrary::CalculateCameraRotation(CameraVolume->CameraLocation, CameraVolume->CameraFocalPoint, CameraVolume->CameraRoll);
		}

		// Correct camera location and rotation according to volume rotation
		if (!CameraVolume->GetActorRotation().IsZero())
		{
			if (CameraVolume->GetIsCameraStatic())
				NewCameraLocation = CameraVolume->GetActorLocation() + CameraVolume->GetActorQuat().RotateVector(NewCameraLocation - CameraVolume->GetActorLocation());
			else
				NewCameraLocation = PlayerPawnLocation + CameraVolume->GetActorQuat().RotateVector(NewCameraLocation - PlayerPawnLocation);

			NewCameraRotation = CameraVolume->GetActorQuat() * NewCameraRotation;
		}
	}

	if (CameraComponent->bUseAdditionalCameraParams)
	{
		NewCameraLocation += CameraComponent->AdditionalCameraLocation;
		NewCameraRotation *= CameraComponent->AdditionalCameraRotation.Quaternion();

		if (bIsCameraOrthographic)
			NewCameraFOV_OW += CameraComponent->AdditionalCameraOrthoWidth;
		else
			NewCameraFOV_OW += CameraComponent->AdditionalCameraFOV;
	}

	if (bNeedsSmoothTransition)
	{
		SmoothTransitionAlpha += DeltaTime * SmoothTransitionSpeed;
		if (SmoothTransitionAlpha <= 1.f)
		{
			NewCameraLocation = FMath::Lerp(OldCameraLocation, NewCameraLocation, SmoothTransitionAlpha);
			NewCameraRotation = FQuat::Slerp(OldCameraRotation, NewCameraRotation, SmoothTransitionAlpha);
			NewCameraFOV_OW = FMath::Lerp(OldCameraFOV_OW, NewCameraFOV_OW, SmoothTransitionAlpha);
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
	}
	else
	{
		if (CameraComponent->bEnableCameraLocationLag)
			NewCameraLocation = FMath::VInterpTo(OldCameraLocation, NewCameraLocation, DeltaTime, CameraComponent->CameraLocationLagSpeed);

		if (CameraComponent->bEnableCameraRotationLag)
			NewCameraRotation = FMath::QInterpTo(OldCameraRotation, NewCameraRotation, DeltaTime, CameraComponent->CameraRotationLagSpeed);

		if (bIsCameraOrthographic)
		{
			if (CameraComponent->bEnableCameraOrthoWidthInterp)
				NewCameraFOV_OW = FMath::FInterpTo(OldCameraFOV_OW, NewCameraFOV_OW, DeltaTime, CameraComponent->CameraOrthoWidthInterpSpeed);
		}
		else
		{
			if (CameraComponent->bEnableCameraFOVInterp)
				NewCameraFOV_OW = FMath::FInterpTo(OldCameraFOV_OW, NewCameraFOV_OW, DeltaTime, CameraComponent->CameraFOVInterpSpeed);
		}
	}
}

void ACameraVolumesCameraManager::SetTransitionBySideInfo(ACameraVolumeActor* CameraVolume, ESide Side)
{
	FSideInfo SideInfo = CameraVolume->GetSideInfo(Side);
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
}

FVector ACameraVolumesCameraManager::CalculateScreenWorldExtentAtDepth(float Depth)
{
	FVector ScreenExtentResult = FVector::ZeroVector;
	APawn* PlayerPawn = GetOwningPlayerController()->GetPawn();
	if (PlayerPawn)
	{
		ICameraVolumesCharacterInterface* PlayerCharacter = Cast<ICameraVolumesCharacterInterface>(PlayerPawn);
		if (PlayerCharacter)
		{
			UCameraVolumesCameraComponent* PlayerCameraComponent = PlayerCharacter->GetCameraComponent();
			float CameraFOV_OW;

			if (PlayerCameraComponent->GetIsCameraOrthographic())
			{
				CameraFOV_OW = PlayerCameraComponent->OrthoWidth;
				ScreenExtentResult.X = CameraFOV_OW * 0.5f;
			}
			else
			{
				CameraFOV_OW = PlayerCameraComponent->FieldOfView;
				ScreenExtentResult.X = FMath::Abs((GetCameraLocation().Y - Depth) * FMath::Tan(FMath::DegreesToRadians(NewCameraFOV_OW * 0.5f))); //Side-scroller
				//ScreenExtentResult.X = FMath::Abs((GetCameraLocation().Z - Depth) * FMath::Tan(FMath::DegreesToRadians(NewCameraFOV_OW * 0.5f))); //Top-down
			}

			float ScreenAspectRatio;
			if (GEngine->GameViewport->Viewport)
			{
				const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
				ScreenAspectRatio = ViewportSize.X / ViewportSize.Y;
			}
			else
				ScreenAspectRatio = CameraComponent->AspectRatio;

			ScreenExtentResult = FVector(ScreenExtentResult.X, 0.f, ScreenExtentResult.X / ScreenAspectRatio); //Side-scroller
			//ScreenExtentResult = FVector(ScreenExtentResult.X, ScreenExtentResult.X / ScreenAspectRatio, 0.f); //Top-down
		}
	}

	return ScreenExtentResult;
}