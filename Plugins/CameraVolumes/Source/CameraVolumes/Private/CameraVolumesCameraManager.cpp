//Dmitriy Barannik aka redbox, 2019

#include "CameraVolumesCameraManager.h"

ACameraVolumesCameraManager::ACameraVolumesCameraManager()
{
	bUpdateCamera = true;
	bCheckCameraVolumes = true;
	PlayerPawn = nullptr;
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

void ACameraVolumesCameraManager::SetUpdateCamera(bool bNewUpdateCamera)
{
	bUpdateCamera = bNewUpdateCamera;
}

void ACameraVolumesCameraManager::UpdateCamera(float DeltaTime)
{
	if (bUpdateCamera)
	{
		PlayerPawn = Cast<ACameraVolumesCharacter>(GetOwningPlayerController()->GetPawn());
		if (PlayerPawn)
		{
			PlayerPawnLocation = PlayerPawn->GetActorLocation();

			// Prepare params
			CamVolPrevious = CamVolCurrent;
			CamVolCurrent = nullptr;
			OldCameraLocation = NewCameraLocation;
			NewCameraLocation = PlayerPawn->DefaultCameraLocation + PlayerPawnLocation;
			OldCameraRotation = NewCameraRotation;
			NewCameraRotation = PlayerPawn->DefaultCameraRotation;
			OldCameraFOV = NewCameraFOV;
			NewCameraFOV = PlayerPawn->DefaultCameraFieldOfView;

			if (bCheckCameraVolumes)
			{
				// Try to get overlapping camera volumes stored in pawn
				if (PlayerPawn->OverlappingCameraVolumes.Num() > 0)
					CamVolCurrent = UCameraVolumesFunctionLibrary::GetCurrentCameraVolume(PlayerPawn->OverlappingCameraVolumes, PlayerPawnLocation);
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
							ACameraVolumeActor* CamVol = Cast<ACameraVolumeActor>(Actor);
							if (CamVol)
								OverlappingCameraVolumes.Add(CamVol);
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

			PlayerPawn->UpdateCamera(NewCameraLocation, NewCameraRotation, NewCameraFOV);
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

		float PlayerCamFOVTangens = FMath::Tan((PI / (180.f)) * (NewCameraFOV * 0.5f));
		float PlayerCamAspectRatio = PlayerPawn->GetCameraComponent()->AspectRatio;

		// Location and Rotation
		if (CameraVolume->GetIsCameraStatic())
		{
			NewCameraLocation = CameraVolume->GetActorLocation() + CameraVolume->CameraLocation;

			if (CameraVolume->bFocalPointIsPlayer)
				NewCameraRotation = FRotationMatrix::MakeFromX(PlayerPawn->GetActorLocation() - CameraVolume->GetActorLocation() - CameraVolume->CameraLocation + CameraVolume->CameraFocalPoint).ToQuat();
			else
				NewCameraRotation = CameraVolume->CameraRotation;
		}
		else
		{
			FVector DeltaExtent = FVector::ZeroVector;
			// Side-scroller
			DeltaExtent.Y = FMath::Abs(PlayerCamFOVTangens * CameraVolume->CamVolWorldMaxCorrected.X);
			DeltaExtent = FVector(0.f, DeltaExtent.Y, DeltaExtent.Y / PlayerCamAspectRatio);
			// Top-down
			//DeltaExtent.Y = FMath::Abs(PlayerCamFOVTangens * CameraVolume->CamVolWorldMaxCorrected.Z);
			//DeltaExtent = FVector(DeltaExtent.Y / PlayerCamAspectRatio, DeltaExtent.Y, 0.f);
			FVector NewCamVolExtentCorrected = CameraVolume->CamVolExtentCorrected + DeltaExtent;
			FVector NewCamVolWorldMinCorrected = CameraVolume->CamVolWorldMinCorrected - DeltaExtent;
			FVector NewCamVolWorldMaxCorrected = CameraVolume->CamVolWorldMaxCorrected + DeltaExtent;

			if (CameraVolume->bOverrideCameraLocation)
			{
				NewCameraLocation = PlayerPawnLocation + CameraVolume->CameraLocation;

				if (CameraVolume->bCameraLocationRelativeToVolume)
					NewCameraLocation.X = CameraVolume->GetActorLocation().X + CameraVolume->CameraLocation.X; // Side-scroller
				//NewCameraLocation.Z = CameraVolume->GetActorLocation().Z + CameraVolume->CameraLocation.Z; // Top-down
			}

			if (CameraVolume->CamVolAspectRatio >= PlayerCamAspectRatio) // Horizontal movement
				// Side-scroller
				NewCameraLocation.X = FMath::Clamp(NewCameraLocation.X, NewCameraLocation.X, NewCamVolExtentCorrected.Z * PlayerCamAspectRatio / PlayerCamFOVTangens);
			// Top-down
			//NewCameraLocation.Z = FMath::Clamp(NewCameraLocation.Z, NewCameraLocation.Z, NewCamVolExtentCorrected.X * PlayerCamAspectRatio / PlayerCamFOVTangens);
			else // Vertical movement
				NewCameraLocation.X = FMath::Clamp(NewCameraLocation.X, NewCameraLocation.X, NewCamVolExtentCorrected.Y / PlayerCamFOVTangens);

			// Calculate screen world extent at depth
			FVector ScreenExtent = FVector::ZeroVector;
			// Side-scroller
			ScreenExtent.Y = FMath::Abs(PlayerCamFOVTangens * NewCameraLocation.X);
			ScreenExtent = FVector(0.f, ScreenExtent.Y, ScreenExtent.Y / PlayerCamAspectRatio);
			// Top-down
			//ScreenExtent.Y = FMath::Abs(PlayerCamFOVTangens * NewCameraLocation.Z);
			//ScreenExtent = FVector(ScreenExtent.Y / PlayerCamAspectRatio, ScreenExtent.Y, 0.f);
			FVector ScreenWorldMin = (NewCamVolWorldMinCorrected + ScreenExtent);
			FVector ScreenWorldMax = (NewCamVolWorldMaxCorrected - ScreenExtent);

			// New camera location
			// Side-scroller
			NewCameraLocation = FVector(
				NewCameraLocation.X,
				FMath::Clamp(NewCameraLocation.Y, ScreenWorldMin.Y, ScreenWorldMax.Y),
				FMath::Clamp(NewCameraLocation.Z, ScreenWorldMin.Z, ScreenWorldMax.Z));
			// Top-down
			//NewCameraLocation = FVector(
			//	FMath::Clamp(NewCameraLocation.X, ScreenWorldMin.X, ScreenWorldMax.X),
			//	FMath::Clamp(NewCameraLocation.Y, ScreenWorldMin.Y, ScreenWorldMax.Y),
			//	NewCameraLocation.Z);

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
		if (PlayerPawn->bEnableCameraLocationLag)
			NewCameraLocation = FMath::VInterpTo(OldCameraLocation, NewCameraLocation, DeltaTime, PlayerPawn->CameraLocationLagSpeed);
		if (PlayerPawn->bEnableCameraRotationLag)
			NewCameraRotation = FMath::QInterpTo(OldCameraRotation, NewCameraRotation, DeltaTime, PlayerPawn->CameraRotationLagSpeed);
		if (PlayerPawn->bEnableCameraFOVInterpolation)
			NewCameraFOV = FMath::FInterpTo(OldCameraFOV, NewCameraFOV, DeltaTime, PlayerPawn->CameraFOVInterpolationSpeed);
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
