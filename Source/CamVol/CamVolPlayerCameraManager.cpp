// Dmitriy Barannik aka redbox, 2019

#include "CamVolPlayerCameraManager.h"
#include "CamVolCharacter.h"

ACamVolPlayerCameraManager::ACamVolPlayerCameraManager()
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

void ACamVolPlayerCameraManager::UpdateCamera(float DeltaTime)
{
	Super::UpdateCamera(DeltaTime);

	PlayerPawn = Cast<ACamVolCharacter>(GetOwningPlayerController()->GetPawn());
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
			// Get active camera volume according to overlapping and priority
			int8 MaxPriorityIndex = -100; // this is limited (-100, 100) in ACameraVolumeActor.Priority

			// Try to get overlapping camera volumes stored in pawn
			if (PlayerPawn->OverlappingCameraVolumes.Num() > 0)
			{
				for (ACameraVolumeActor* CamVol : PlayerPawn->OverlappingCameraVolumes)
				{
					if (CamVol)
					{
						GetCurrentCameraVolume(CamVol, PlayerPawnLocation, MaxPriorityIndex);
					}
				}
			}
			// Try to get camera volumes from actors overlapping pawn
			else
			{
				OverlappingActors.Empty();
				PlayerPawn->GetOverlappingActors(OverlappingActors, ACameraVolumeActor::StaticClass());
				if (OverlappingActors.Num() > 0)
				{
					for (AActor* Actor : OverlappingActors)
					{
						ACameraVolumeActor* CamVol = Cast<ACameraVolumeActor>(Actor);
						if (CamVol)
						{
							GetCurrentCameraVolume(CamVol, PlayerPawnLocation, MaxPriorityIndex);
						}
					}
				}
				else
					bCheckCameraVolumes = false;
				// There is no camera volumes overlapping character at this time,
				// so we don't need this check until player pawn overlap some camera volume.
			}

			if (CamVolCurrent)
			{
				if (CamVolCurrent != CamVolPrevious)
				{
					// We've changed to another volume
					// Get passed side and transition type
					PassedSide = GetNearestVolumeSide(CamVolCurrent, PlayerPawnLocation, AllowedDist);
					if (PassedSide == ESide::ES_Unknown)
					{
						// Maybe we've passed not near any side?
						if (CamVolPrevious)
						{
							// Use settings of side we have passed from
							PassedSide = GetNearestVolumeSide(CamVolPrevious, PlayerPawnLocation, AllowedDist);
							SetTransitionBySideInfo(CamVolPrevious, PassedSide);
						}
					}
					else
					{
						SetTransitionBySideInfo(CamVolCurrent, PassedSide);
					}
				}

				CalcNewCameraParams(CamVolCurrent);
				CalcTransitionAndLag(DeltaTime);
			}
			else
			{ // Maybe we've passed from volume into void?
				if (CamVolPrevious)
				{
					// Use settings of side we've passed from
					PassedSide = GetNearestVolumeSide(CamVolPrevious, PlayerPawnLocation, AllowedDist);
					SetTransitionBySideInfo(CamVolPrevious, PassedSide);
					CalcTransitionAndLag(DeltaTime);
				}
			}
		}

		if (bNeedsSmoothTransition)
		{
			CalcTransitionAndLag(DeltaTime);
		}

		PlayerPawn->UpdateCamera(NewCameraLocation, NewCameraRotation, NewCameraFOV);
	}
}

void ACamVolPlayerCameraManager::GetCurrentCameraVolume(ACameraVolumeActor* CamVol, FVector& PlayerPawnLocation, int8 MaxPriorityIndex)
{
	if (CamVol->CamVolWorldMin.X < PlayerPawnLocation.X &&
		PlayerPawnLocation.X < CamVol->CamVolWorldMax.X &&
		CamVol->CamVolWorldMin.Y < PlayerPawnLocation.Y &&
		PlayerPawnLocation.Y < CamVol->CamVolWorldMax.Y &&
		CamVol->CamVolWorldMin.Z < PlayerPawnLocation.Z &&
		PlayerPawnLocation.Z < CamVol->CamVolWorldMax.Z)
	{
		if (CamVol->Priority > MaxPriorityIndex)
		{
			MaxPriorityIndex = CamVol->Priority;
			CamVolCurrent = CamVol;
		}
	}
}

ESide ACamVolPlayerCameraManager::GetNearestVolumeSide(ACameraVolumeActor* CamVol, FVector& PlayerPawnLocation, float Distance)
{
	ESide NearestSide;

	if (FMath::IsNearlyEqual(PlayerPawnLocation.X, CamVol->CamVolWorldMax.X, Distance))
		NearestSide = ESide::ES_Front;
	else if (FMath::IsNearlyEqual(PlayerPawnLocation.X, CamVol->CamVolWorldMin.X, Distance))
		NearestSide = ESide::ES_Back;
	else if (FMath::IsNearlyEqual(PlayerPawnLocation.Y, CamVol->CamVolWorldMin.Y, Distance))
		NearestSide = ESide::ES_Right;
	else if (FMath::IsNearlyEqual(PlayerPawnLocation.Y, CamVol->CamVolWorldMax.Y, Distance))
		NearestSide = ESide::ES_Left;
	else if (FMath::IsNearlyEqual(PlayerPawnLocation.Z, CamVol->CamVolWorldMax.Z, Distance))
		NearestSide = ESide::ES_Top;
	else if (FMath::IsNearlyEqual(PlayerPawnLocation.Z, CamVol->CamVolWorldMin.Z, Distance))
		NearestSide = ESide::ES_Bottom;
	else
		NearestSide = ESide::ES_Unknown;

	return NearestSide;
}

void ACamVolPlayerCameraManager::CalcNewCameraParams(ACameraVolumeActor* CamVol)
{
	// Check is FOV overriden
	if (CamVol->bOverrideCameraFieldOfView)
	{
		NewCameraFOV = CamVol->CameraFieldOfView;
	}

	float PlayerCamFOVTangens = FMath::Tan((PI / (180.f)) * (NewCameraFOV * 0.5f));
	float PlayerCamAspectRatio = PlayerPawn->GetCameraComponent()->AspectRatio;

	// Calculate new location and rotation
	if (CamVol->bFixedCamera)
	{
		NewCameraLocation = CamVol->GetActorLocation() + CamVol->FixedCameraLocation;
		if (CamVol->bFocalPointIsPlayer)
		{
			NewCameraRotation = FRotationMatrix::MakeFromX(PlayerPawn->GetActorLocation() - CamVol->GetActorLocation() - CamVol->FixedCameraLocation).Rotator();
		}
		else
		{
			NewCameraRotation = CamVol->FixedCameraRotation;
		}
	}
	else
	{
		FVector DeltaExtent = FVector::ZeroVector;
		// Side-scroller
		DeltaExtent.Y = FMath::Abs(PlayerCamFOVTangens * CamVol->CamVolWorldMaxCorrected.X);
		DeltaExtent = FVector(0.f, DeltaExtent.Y, DeltaExtent.Y / PlayerCamAspectRatio);
		// Top-down
		//DeltaExtent.Y = FMath::Abs(PlayerCamFOVTangens * CamVol->CamVolWorldMaxCorrected.Z);
		//DeltaExtent = FVector(DeltaExtent.Y / PlayerCamAspectRatio, DeltaExtent.Y, 0.f);
		FVector NewCamVolExtentCorrected = CamVol->CamVolExtentCorrected + DeltaExtent;
		FVector NewCamVolWorldMinCorrected = CamVol->CamVolWorldMinCorrected - DeltaExtent;
		FVector NewCamVolWorldMaxCorrected = CamVol->CamVolWorldMaxCorrected + DeltaExtent;

		// Calculate camera offset according to volume extents
		float NewCameraOffset;
		NewCameraOffset = NewCameraLocation.X; // Side-scroller
		//NewCameraOffset = NewCameraLocation.Z; // Top-down
		if (CamVol->bOverrideCameraOffset)
			NewCameraOffset = CamVol->CameraOffset;

		if (CamVol->CamVolAspectRatio >= PlayerCamAspectRatio)
		{// Horizontal movement
			// Side-scroller
			NewCameraOffset = FMath::Clamp(NewCameraOffset, 0.f, NewCamVolExtentCorrected.Z * PlayerCamAspectRatio / PlayerCamFOVTangens);
			// Top-down
			//NewCameraOffset = FMath::Clamp(NewCameraOffset, 0.f, NewCamVolExtentCorrected.X * PlayerCamAspectRatio / PlayerCamFOVTangens);
		}
		else
		{// Vertical movement
			NewCameraOffset = FMath::Clamp(NewCameraOffset, 0.f, NewCamVolExtentCorrected.Y / PlayerCamFOVTangens);
		}

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

void ACamVolPlayerCameraManager::SetTransitionBySideInfo(ACameraVolumeActor* CamVol, ESide Side)
{
	FSideInfo SideInfo = CamVol->GetSideInfo(Side);
	if (SideInfo.SideTransitionType == ESideTransitionType::ESTT_Smooth)
	{
		bNeedsSmoothTransition = true;
		SmoothTransitionAlpha = 0.f;
		SmoothTransitionTime = CamVol->CameraSmoothTransitionTime;
	}
	else
		bNeedsCutTransition = true;
}

void ACamVolPlayerCameraManager::CalcTransitionAndLag(float DeltaTime)
{
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
		NewCameraFOV = FMath::FInterpTo(OldCameraFOV, NewCameraFOV, DeltaTime, 10.f);
	}
}