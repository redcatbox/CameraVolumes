//Dmitriy Barannik aka redbox, 2019

#include "CameraVolumesCameraManager.h"

ACameraVolumesCameraManager::ACameraVolumesCameraManager()
{
	bUpdateCamera = true;
	bCheckCameraVolumes = true;
	PlayerCharacter = nullptr;
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
		PlayerCharacter = Cast<ACameraVolumesCharacter>(GetOwningPlayerController()->GetPawn());
		if (PlayerCharacter)
		{
			PlayerCharacterLocation = PlayerCharacter->GetActorLocation();

			// Prepare params
			CamVolPrevious = CamVolCurrent;
			CamVolCurrent = nullptr;
			OldCameraLocation = NewCameraLocation;
			NewCameraLocation = PlayerCharacterLocation + PlayerCharacter->DefaultCameraLocation;
			OldCameraRotation = NewCameraRotation;
			NewCameraRotation = PlayerCharacter->DefaultCameraRotation;
			OldCameraFOV = NewCameraFOV;
			NewCameraFOV = PlayerCharacter->DefaultCameraFieldOfView;

			if (bCheckCameraVolumes)
			{
				// Try to get overlapping camera volumes stored in pawn
				if (PlayerCharacter->OverlappingCameraVolumes.Num() > 0)
					CamVolCurrent = UCameraVolumesFunctionLibrary::GetCurrentCameraVolume(PlayerCharacter->OverlappingCameraVolumes, PlayerCharacterLocation);
				// Try to get camera volumes from actors overlapping pawn
				else
				{
					OverlappingActors.Empty();
					PlayerCharacter->GetOverlappingActors(OverlappingActors, ACameraVolumeActor::StaticClass());
					if (OverlappingActors.Num() > 0)
					{
						TArray<ACameraVolumeActor*> OverlappingCameraVolumes;
						for (AActor* Actor : OverlappingActors)
						{
							ACameraVolumeActor* CamVol = Cast<ACameraVolumeActor>(Actor);
							if (CamVol)
								OverlappingCameraVolumes.Add(CamVol);
						}
						CamVolCurrent = UCameraVolumesFunctionLibrary::GetCurrentCameraVolume(OverlappingCameraVolumes, PlayerCharacterLocation);
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
						PassedSideCurrent = CamVolCurrent->GetNearestVolumeSide(PlayerCharacterLocation);

						if (CamVolPrevious)
						{
							ESide PassedSidePrevious;
							PassedSidePrevious = CamVolPrevious->GetNearestVolumeSide(PlayerCharacterLocation);

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
					ESide PassedSidePrevious = CamVolPrevious->GetNearestVolumeSide(PlayerCharacterLocation);
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

			PlayerCharacter->UpdateCamera(NewCameraLocation, NewCameraRotation, NewCameraFOV);
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
		float ScreenAspectRatio;
		if (GEngine)
		{
			const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
			ScreenAspectRatio = ViewportSize.X / ViewportSize.Y;
		}
		else
			ScreenAspectRatio = PlayerCharacter->GetCameraComponent()->AspectRatio;

		// Location and Rotation
		if (CameraVolume->GetIsCameraStatic())
		{
			NewCameraLocation = CameraVolume->GetActorLocation() + CameraVolume->CameraLocation;

			if (CameraVolume->bFocalPointIsPlayer)
				NewCameraRotation = FRotationMatrix::MakeFromX(PlayerCharacter->GetActorLocation() - CameraVolume->GetActorLocation() - CameraVolume->CameraLocation + CameraVolume->CameraFocalPoint).ToQuat();
			else
				NewCameraRotation = CameraVolume->CameraRotation;
		}
		else
		{
			// Camera offset is always relative to CameraVolume->GetActorLocation().X
			float CameraOffset;

			if (CameraVolume->bOverrideCameraLocation)
			{
				NewCameraLocation = PlayerCharacterLocation + CameraVolume->CameraLocation;

				if (CameraVolume->bCameraLocationRelativeToVolume)
				{
					CameraOffset = CameraVolume->CameraLocation.X; // Side-scroller
					//CameraOffset = CameraVolume->CameraLocation.Z; // Top-down
				}
				else
				{
					CameraOffset = NewCameraLocation.X - CameraVolume->GetActorLocation().X; // Side-scroller
					//CameraOffset = PlayerCharacterLocation.Z - CameraVolume->GetActorLocation().Z; // Top-down
				}
			}
			else
				CameraOffset = NewCameraLocation.X - CameraVolume->GetActorLocation().X; // Side-scroller
				//CameraOffset = NewCameraLocation.Z - CameraVolume->GetActorLocation().Z; // Top-down

			// Calculate delta volume extent with +X volume extent
			FVector DeltaExtent = FVector::ZeroVector;
			// Side-scroller
			DeltaExtent.Y = FMath::Abs(CameraVolume->CamVolWorldMaxCorrected.X * PlayerCamFOVTangens);
			DeltaExtent = FVector(0.f, DeltaExtent.Y, DeltaExtent.Y / ScreenAspectRatio);
			// Top-down
			//DeltaExtent.Y = FMath::Abs(PlayerCamFOVTangens * CameraVolume->CamVolWorldMaxCorrected.Z);
			//DeltaExtent = FVector(DeltaExtent.Y / ScreenAspectRatio, DeltaExtent.Y, 0.f);
			FVector NewCamVolExtentCorrected = CameraVolume->CamVolExtentCorrected + DeltaExtent;
			FVector NewCamVolWorldMinCorrected = CameraVolume->CamVolWorldMinCorrected - DeltaExtent;
			FVector NewCamVolWorldMaxCorrected = CameraVolume->CamVolWorldMaxCorrected + DeltaExtent;

			if (CameraVolume->CamVolAspectRatio >= ScreenAspectRatio) // Horizontal movement
				// Side-scroller
				CameraOffset = FMath::Clamp(CameraOffset, CameraOffset, NewCamVolExtentCorrected.Z * ScreenAspectRatio / PlayerCamFOVTangens);
			// Top-down
			//CameraOffset = FMath::Clamp(CameraOffset, CameraOffset, NewCamVolExtentCorrected.X * ScreenAspectRatio / PlayerCamFOVTangens);
			else // Vertical movement
				CameraOffset = FMath::Clamp(CameraOffset, CameraOffset, NewCamVolExtentCorrected.Y / PlayerCamFOVTangens);

			// Calculate screen world extent at depth
			FVector ScreenExtent = FVector::ZeroVector;
			ScreenExtent.Y = FMath::Abs(CameraOffset * PlayerCamFOVTangens); // Side-scroller
			ScreenExtent = FVector(0.f, ScreenExtent.Y, ScreenExtent.Y / ScreenAspectRatio);
			//ScreenExtent = FVector(ScreenExtent.Y / ScreenAspectRatio, ScreenExtent.Y, 0.f); // Top-down
			FVector ScreenWorldMin = (NewCamVolWorldMinCorrected + ScreenExtent);
			FVector ScreenWorldMax = (NewCamVolWorldMaxCorrected - ScreenExtent);

			// New camera location
			// Side-scroller
			NewCameraLocation = FVector(
				CameraOffset + CameraVolume->GetActorLocation().X,
				FMath::Clamp(NewCameraLocation.Y, ScreenWorldMin.Y, ScreenWorldMax.Y),
				FMath::Clamp(NewCameraLocation.Z, ScreenWorldMin.Z, ScreenWorldMax.Z));
			// Top-down
			//NewCameraLocation = FVector(
			//	FMath::Clamp(NewCameraLocation.X, ScreenWorldMin.X, ScreenWorldMax.X),
			//	FMath::Clamp(NewCameraLocation.Y, ScreenWorldMin.Y, ScreenWorldMax.Y),
			//	CameraOffset + CameraVolume->GetActorLocation().Z);

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
		if (PlayerCharacter->bEnableCameraLocationLag)
			NewCameraLocation = FMath::VInterpTo(OldCameraLocation, NewCameraLocation, DeltaTime, PlayerCharacter->CameraLocationLagSpeed);
		if (PlayerCharacter->bEnableCameraRotationLag)
			NewCameraRotation = FMath::QInterpTo(OldCameraRotation, NewCameraRotation, DeltaTime, PlayerCharacter->CameraRotationLagSpeed);
		if (PlayerCharacter->bEnableCameraFOVInterpolation)
			NewCameraFOV = FMath::FInterpTo(OldCameraFOV, NewCameraFOV, DeltaTime, PlayerCharacter->CameraFOVInterpolationSpeed);
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
