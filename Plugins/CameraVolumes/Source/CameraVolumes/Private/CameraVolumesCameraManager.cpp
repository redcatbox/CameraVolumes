//Dmitriy Barannik aka redbox, 2019

#include "CameraVolumesCameraManager.h"
#include "GameFramework/PlayerController.h"

ACameraVolumesCameraManager::ACameraVolumesCameraManager()
{
	bUpdateCamera = true;
	bCheckCameraVolumes = true;
	PlayerCharacter = nullptr;
	PlayerPaperCharacter = nullptr;
	CamVolPrevious = nullptr;
	CamVolCurrent = nullptr;
	OldCameraLocation = FVector::ZeroVector;
	NewCameraLocation = FVector::ZeroVector;
	OldCameraRotation = FQuat();
	NewCameraRotation = FQuat();
	OldCameraFOV = 90.f;
	NewCameraFOV = 90.f;
	bNeedsSmoothTransition = false;
	SmoothTransitionAlpha = 0.f;
	bNeedsCutTransition = false;
}

void ACameraVolumesCameraManager::SetCheckCameraVolumes(bool bNewCheck)
{
	bCheckCameraVolumes = bNewCheck;
}

void ACameraVolumesCameraManager::SetUpdateCamera(bool bNewUpdateCamera)
{
	bUpdateCamera = bNewUpdateCamera;
}

void ACameraVolumesCameraManager::UpdateCamera(float DeltaTime)
{

	if (bTestConfig)
	{
		UE_LOG(LogTemp, Log, TEXT("bTestConfig"));
	}

	if (bUpdateCamera)
	{
		bool bCharacterValid = false;
		APawn* PlayerPawn = GetOwningPlayerController()->GetPawn();
		if (PlayerPawn)
		{
			PlayerCharacter = Cast<ACameraVolumesCharacter>(PlayerPawn);
			if (PlayerCharacter)
			{
				bCharacterValid = true;
				CameraComponent = PlayerCharacter->GetCameraComponent();
			}
			else
			{
				PlayerPaperCharacter = Cast<ACameraVolumesPaperCharacter>(PlayerPawn);
				if (PlayerPaperCharacter)
				{
					bCharacterValid = true;
					CameraComponent = PlayerPaperCharacter->GetCameraComponent();
				}
			}
		}

		if (bCharacterValid)
		{
			PlayerPawnLocation = PlayerPawn->GetActorLocation();

			// Prepare params
			CamVolPrevious = CamVolCurrent;
			CamVolCurrent = nullptr;
			OldCameraLocation = NewCameraLocation;
			NewCameraLocation = PlayerPawnLocation + CameraComponent->DefaultCameraLocation;
			OldCameraRotation = NewCameraRotation;
			NewCameraRotation = CameraComponent->DefaultCameraRotation;
			OldCameraFOV = NewCameraFOV;
			NewCameraFOV = CameraComponent->DefaultCameraFieldOfView;

			if (bCheckCameraVolumes)
			{
				// Try to get overlapping camera volumes stored in pawn
				if (CameraComponent->OverlappingCameraVolumes.Num() > 0)
					CamVolCurrent = UCameraVolumesFunctionLibrary::GetCurrentCameraVolume(CameraComponent->OverlappingCameraVolumes, PlayerPawnLocation);
				// Try to get camera volumes from actors overlapping pawn
				else
				{
					OverlappingActors.Empty();
					PlayerPawn->GetOverlappingActors(OverlappingActors, ACameraVolumeActor::StaticClass());
					if (OverlappingActors.Num() > 0)
					{
						TArray<ACameraVolumeActor*> OverlappingCameraVolumes;
						for (AActor* Actor : OverlappingActors)
						{
							ACameraVolumeActor* CameraVolume = Cast<ACameraVolumeActor>(Actor);
							if (CameraVolume)
								OverlappingCameraVolumes.Add(CameraVolume);
						}
						CamVolCurrent = UCameraVolumesFunctionLibrary::GetCurrentCameraVolume(OverlappingCameraVolumes, PlayerPawnLocation);
					}
					else
						bCheckCameraVolumes = false;
					// There is no camera volumes overlapping character at this time,
					// so we don't need this check until player pawn overlap some camera volume again.
				}

				if (CamVolCurrent)
				{
					if (CamVolCurrent != CamVolPrevious) // Do we changed to another volume?
					{
						// Check for dynamic camera volumes
						ACameraVolumeDynamicActor* CamVolCurrentDynamic = Cast<ACameraVolumeDynamicActor>(CamVolCurrent);
						if (CamVolCurrentDynamic)
							CamVolCurrentDynamic->SetActive(true);
						ACameraVolumeDynamicActor* CamVolPreviousDynamic = Cast<ACameraVolumeDynamicActor>(CamVolPrevious);
						if (CamVolPreviousDynamic)
							CamVolPreviousDynamic->SetActive(false);

						ESide PassedSideCurrent;
						PassedSideCurrent = CamVolCurrent->GetNearestVolumeSide(PlayerPawnLocation);

						if (CamVolPrevious)
						{
							ESide PassedSidePrevious;
							PassedSidePrevious = CamVolPrevious->GetNearestVolumeSide(PlayerPawnLocation);

							if (UCameraVolumesFunctionLibrary::CompareSidesPairs(PassedSideCurrent, PassedSidePrevious))
								// we've passed to nearby volume
								// Use settings of side we have passed to
								SetTransitionBySideInfo(CamVolCurrent, PassedSideCurrent);
							else
							{
								// we've passed to volume with another priority
								if (CamVolCurrent->Priority > CamVolPrevious->Priority)
									// Use settings of side we have passed to
									SetTransitionBySideInfo(CamVolCurrent, PassedSideCurrent);
								else
									// Use settings of side we have passed from
									SetTransitionBySideInfo(CamVolPrevious, PassedSidePrevious);
							}
						}
						else
							// we've passed from void to volume
							SetTransitionBySideInfo(CamVolCurrent, PassedSideCurrent);
					}

					CalcNewCameraParams(CamVolCurrent, DeltaTime);
				}
				else if (CamVolPrevious) // Do we passed from volume to void?
				{
					// Check for dynamic camera volumes
					ACameraVolumeDynamicActor* CamVolPreviousDynamic = Cast<ACameraVolumeDynamicActor>(CamVolPrevious);
					if (CamVolPreviousDynamic)
						CamVolPreviousDynamic->SetActive(false);

					// Use settings of side we've passed from
					ESide PassedSidePrevious = CamVolPrevious->GetNearestVolumeSide(PlayerPawnLocation);
					SetTransitionBySideInfo(CamVolPrevious, PassedSidePrevious);
					CalcNewCameraParams(nullptr, DeltaTime);
				}
				else
				{
					CalcNewCameraParams(nullptr, DeltaTime);
				}
			}
			else
				CalcNewCameraParams(nullptr, DeltaTime);

			CameraComponent->UpdateCamera(NewCameraLocation, NewCameraRotation, NewCameraFOV);
		}
	}

	Super::UpdateCamera(DeltaTime);
}

void ACameraVolumesCameraManager::CalcNewCameraParams(ACameraVolumeActor* CameraVolume, float DeltaTime)
{
	if (CameraVolume)
	{
		// FOV
		if (CameraVolume->bOverrideCameraFieldOfView)
			NewCameraFOV = CameraVolume->CameraFieldOfView;

		// Get screen (or at least camera) aspect ratio for further calculations
		//float PlayerCamFOVTangens = FMath::Tan((PI / (180.f)) * (NewCameraFOV * 0.5f));
		float PlayerCamFOVTangens = FMath::Tan(FMath::DegreesToRadians(NewCameraFOV * 0.5f));
		float ScreenAspectRatio;
		if (GEngine)
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
				NewCameraRotation = UCameraVolumesFunctionLibrary::CalculateCameraRotation(CameraVolume->CameraLocation, CameraVolume->CameraFocalPoint, CameraVolume->CameraRoll);
			else
				NewCameraRotation = CameraVolume->CameraRotation;
		}
		else
		{
			// Camera offset is always relative to CameraVolume->GetActorLocation().X
			float CameraOffset;

			if (CameraVolume->bOverrideCameraLocation)
			{
				NewCameraLocation = PlayerPawnLocation + CameraVolume->CameraLocation;

				if (CameraVolume->bCameraLocationRelativeToVolume)
				{
					if (CameraVolume->GetIsCameraSideScroller())
						CameraOffset = CameraVolume->CameraLocation.X;
					else
						CameraOffset = CameraVolume->CameraLocation.Z;
				}
				else
				{
					if (CameraVolume->GetIsCameraSideScroller())
						CameraOffset = NewCameraLocation.X - CameraVolume->GetActorLocation().X;
					else
						CameraOffset = PlayerPawnLocation.Z - CameraVolume->GetActorLocation().Z;
				}
			}
			else
			{
				if (CameraVolume->GetIsCameraSideScroller())
					CameraOffset = NewCameraLocation.X - CameraVolume->GetActorLocation().X;
				else
					CameraOffset = NewCameraLocation.Z - CameraVolume->GetActorLocation().Z;
			}


			FVector NewCamVolExtentCorrected = CameraVolume->CamVolExtentCorrected;
			FVector NewCamVolWorldMinCorrected = CameraVolume->CamVolWorldMinCorrected;
			FVector NewCamVolWorldMaxCorrected = CameraVolume->CamVolWorldMaxCorrected;
			if (!CameraVolume->bUseZeroDepthExtent)
			{
				// Calculate delta volume extent with +X volume extent
				FVector DeltaExtent = FVector::ZeroVector;
				if (CameraVolume->GetIsCameraSideScroller())
				{
					DeltaExtent.Y = FMath::Abs(CameraVolume->CamVolWorldMaxCorrected.X * PlayerCamFOVTangens);
					DeltaExtent = FVector(0.f, DeltaExtent.Y, DeltaExtent.Y / ScreenAspectRatio);
				}
				else
				{
					DeltaExtent.Y = FMath::Abs(CameraVolume->CamVolWorldMaxCorrected.Z * PlayerCamFOVTangens);
					DeltaExtent = FVector(DeltaExtent.Y / ScreenAspectRatio, DeltaExtent.Y, 0.f);
				}

				NewCamVolExtentCorrected = CameraVolume->CamVolExtentCorrected + DeltaExtent;
				NewCamVolWorldMinCorrected = CameraVolume->CamVolWorldMinCorrected - DeltaExtent;
				NewCamVolWorldMaxCorrected = CameraVolume->CamVolWorldMaxCorrected + DeltaExtent;
			}

			if (CameraVolume->CamVolAspectRatio >= ScreenAspectRatio) // Horizontal movement
			{
				if (CameraVolume->GetIsCameraSideScroller())
					CameraOffset = FMath::Clamp(CameraOffset, CameraOffset, NewCamVolExtentCorrected.Z * ScreenAspectRatio / PlayerCamFOVTangens);
				else
					CameraOffset = FMath::Clamp(CameraOffset, CameraOffset, NewCamVolExtentCorrected.X * ScreenAspectRatio / PlayerCamFOVTangens);
			}
			else // Vertical movement
				CameraOffset = FMath::Clamp(CameraOffset, CameraOffset, NewCamVolExtentCorrected.Y / PlayerCamFOVTangens);

			// Calculate screen world extent at depth (CameraOffset)
			FVector ScreenExtent = FVector::ZeroVector;
			ScreenExtent.Y = FMath::Abs(CameraOffset * PlayerCamFOVTangens);
			if (CameraVolume->GetIsCameraSideScroller())
				ScreenExtent = FVector(0.f, ScreenExtent.Y, ScreenExtent.Y / ScreenAspectRatio);
			else
				ScreenExtent = FVector(ScreenExtent.Y / ScreenAspectRatio, ScreenExtent.Y, 0.f);
			FVector ScreenWorldMin = (NewCamVolWorldMinCorrected + ScreenExtent);
			FVector ScreenWorldMax = (NewCamVolWorldMaxCorrected - ScreenExtent);

			// New camera location
			if (CameraVolume->GetIsCameraSideScroller())
				NewCameraLocation = FVector(
					CameraOffset + CameraVolume->GetActorLocation().X,
					FMath::Clamp(NewCameraLocation.Y, ScreenWorldMin.Y, ScreenWorldMax.Y),
					FMath::Clamp(NewCameraLocation.Z, ScreenWorldMin.Z, ScreenWorldMax.Z));
			else
				NewCameraLocation = FVector(
					FMath::Clamp(NewCameraLocation.X, ScreenWorldMin.X, ScreenWorldMax.X),
					FMath::Clamp(NewCameraLocation.Y, ScreenWorldMin.Y, ScreenWorldMax.Y),
					CameraOffset + CameraVolume->GetActorLocation().Z);

			// New camera rotation
			NewCameraRotation = FRotationMatrix::MakeFromX(CameraVolume->CameraFocalPoint - CameraVolume->CameraLocation).ToQuat();
		}
	}

	if (bNeedsSmoothTransition)
	{
		if (SmoothTransitionAlpha <= SmoothTransitionTime)
		{
			SmoothTransitionAlpha += DeltaTime;
			NewCameraLocation = FMath::Lerp(OldCameraLocation, NewCameraLocation, SmoothTransitionAlpha);
			NewCameraRotation = FMath::Lerp(OldCameraRotation, NewCameraRotation, SmoothTransitionAlpha);
			NewCameraFOV = FMath::Lerp(OldCameraFOV, NewCameraFOV, SmoothTransitionAlpha);
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
		if (CameraComponent->bEnableCameraFOVInterpolation)
			NewCameraFOV = FMath::FInterpTo(OldCameraFOV, NewCameraFOV, DeltaTime, CameraComponent->CameraFOVInterpolationSpeed);
	}
}

void ACameraVolumesCameraManager::SetTransitionBySideInfo(ACameraVolumeActor* CameraVolume, ESide Side)
{
	FSideInfo SideInfo = CameraVolume->GetSideInfo(Side);
	if (SideInfo.SideTransitionType == ESideTransitionType::ESTT_Smooth)
	{
		bNeedsSmoothTransition = true;
		SmoothTransitionAlpha = 0.f;
		SmoothTransitionTime = CameraVolume->CameraSmoothTransitionTime;
	}
	else if (SideInfo.SideTransitionType == ESideTransitionType::ESTT_Cut)
		bNeedsCutTransition = true;
}
