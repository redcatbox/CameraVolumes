//Dmitriy Barannik aka redbox, 2019

#include "CameraVolumesCameraManager.h"

ACameraVolumesCameraManager::ACameraVolumesCameraManager()
{
	bCheckCameraVolumes = true;
	PlayerPawn = nullptr;
	CamVolPrevious = nullptr;
	CamVolCurrent = nullptr;
	OldCameraLocation = FVector::ZeroVector;
	NewCameraLocation = FVector::ZeroVector;
	OldCameraRotation = FRotator::ZeroRotator;
	NewCameraRotation = FRotator::ZeroRotator;
	OldCameraFOV = 90.f;
	NewCameraFOV = 90.f;
	bNeedsSmoothTransition = false;
	SmoothTransitionAlpha = 0.f;
	bNeedsCutTransition = false;
}

void ACameraVolumesCameraManager::UpdateCamera(float DeltaTime)
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
		NewCameraFOV = PlayerPawn->DefaultCameraFieldOfView; // Needs to be replaced with game settings value

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

	Super::UpdateCamera(DeltaTime);
}

void ACameraVolumesCameraManager::CalcNewCameraParams(ACameraVolumeActor* CameraVolume, float DeltaTime)
{
	if (CameraVolume)
	{
		// Check is FOV overriden
		if (CameraVolume->bOverrideCameraFieldOfView)
			NewCameraFOV = CameraVolume->CameraFieldOfView;

		float PlayerCamFOVTangens = FMath::Tan((PI / (180.f)) * (NewCameraFOV * 0.5f));
		float PlayerCamAspectRatio = PlayerPawn->GetCameraComponent()->AspectRatio;

		// Calculate new location and rotation
		if (CameraVolume->bFixedCamera)
		{
			NewCameraLocation = CameraVolume->GetActorLocation() + CameraVolume->FixedCameraLocation;

			if (CameraVolume->bFocalPointIsPlayer)
				NewCameraRotation = FRotationMatrix::MakeFromX(PlayerPawn->GetActorLocation() - CameraVolume->GetActorLocation() - CameraVolume->FixedCameraLocation).Rotator();
			else
				NewCameraRotation = CameraVolume->FixedCameraRotation;
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

			// Calculate camera offset according to volume extents
			float NewCameraOffset;
			NewCameraOffset = NewCameraLocation.X; // Side-scroller
			//NewCameraOffset = NewCameraLocation.Z; // Top-down
			if (CameraVolume->bOverrideCameraOffset)
				NewCameraOffset = PlayerPawnLocation.X + CameraVolume->CameraOffset; // Side-scroller
				//NewCameraOffset = PlayerPawnLocation.Z + CameraVolume->CameraOffset; // Top-down

			if (CameraVolume->CamVolAspectRatio >= PlayerCamAspectRatio) // Horizontal movement
				// Side-scroller
				NewCameraOffset = FMath::Clamp(NewCameraOffset, 0.f, NewCamVolExtentCorrected.Z * PlayerCamAspectRatio / PlayerCamFOVTangens);
			// Top-down
			//NewCameraOffset = FMath::Clamp(NewCameraOffset, 0.f, NewCamVolExtentCorrected.X * PlayerCamAspectRatio / PlayerCamFOVTangens);
			else // Vertical movement
				NewCameraOffset = FMath::Clamp(NewCameraOffset, 0.f, NewCamVolExtentCorrected.Y / PlayerCamFOVTangens);

			// Calculate screen world extent at NewCameraOffset
			FVector ScreenExtent = FVector::ZeroVector;
			ScreenExtent.Y = FMath::Abs(PlayerCamFOVTangens * NewCameraOffset);
			ScreenExtent = FVector(0.f, ScreenExtent.Y, ScreenExtent.Y / PlayerCamAspectRatio); // Side-scroller
			//ScreenExtent = FVector(ScreenExtent.Y / PlayerCamAspectRatio, ScreenExtent.Y, 0.f); // Top-down
			FVector ScreenWorldMin = (NewCamVolWorldMinCorrected + ScreenExtent);
			FVector ScreenWorldMax = (NewCamVolWorldMaxCorrected - ScreenExtent);

			// New camera location
			// Side-scroller
			NewCameraLocation = FVector(
				NewCameraOffset,
				FMath::Clamp(PlayerPawnLocation.Y, ScreenWorldMin.Y, ScreenWorldMax.Y),
				FMath::Clamp(PlayerPawnLocation.Z, ScreenWorldMin.Z, ScreenWorldMax.Z));
			// Top-down
			//NewCameraLocation = FVector(
			//	FMath::Clamp(PlayerPawnLocation.X, ScreenWorldMin.X, ScreenWorldMax.X),
			//	FMath::Clamp(PlayerPawnLocation.Y, ScreenWorldMin.Y, ScreenWorldMax.Y),
			//	NewCameraOffset);
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
			NewCameraRotation = FMath::RInterpTo(OldCameraRotation, NewCameraRotation, DeltaTime, PlayerPawn->CameraRotationLagSpeed);
		if (PlayerPawn->bEnableCameraFOVLag)
			NewCameraFOV = FMath::FInterpTo(OldCameraFOV, NewCameraFOV, DeltaTime, PlayerPawn->CameraFOVLagSpeed);
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
