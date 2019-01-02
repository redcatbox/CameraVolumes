// Fill out your copyright notice in the Description page of Project Settings.

#include "CamVolPlayerCameraManager.h"
#include "CamVolCharacter.h"

ACamVolPlayerCameraManager::ACamVolPlayerCameraManager()
{
	bCheckCameraVolumes = true;
	PlayerPawn = nullptr;
	CamVolPrevious = nullptr;
	CamVolCurrent = nullptr;
	OldCameraLocation = FVector::ZeroVector; // if location lag enabled, better to be equal to PlayerPawn->DefaultCameraLocation
	NewCameraLocation = FVector::ZeroVector; // if location lag enabled, better to be equal to PlayerPawn->DefaultCameraLocation
	OldCameraRotation = FRotator::ZeroRotator; // if rotation lag enabled, better to be equal to PlayerPawn->DefaultCameraRotation
	NewCameraRotation = FRotator::ZeroRotator; // if rotation lag enabled, better to be equal to PlayerPawn->DefaultCameraRotation
	OldCameraFOV, NewCameraFOV = 90.f;
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
		FVector PlayerPawnLocation = PlayerPawn->GetActorLocation();

		// Prepare camera params
		OldCameraLocation = NewCameraLocation;
		NewCameraLocation = PlayerPawn->DefaultCameraLocation + PlayerPawnLocation;
		OldCameraRotation = NewCameraRotation;
		NewCameraRotation = PlayerPawn->DefaultCameraRotation;
		OldCameraFOV = NewCameraFOV;
		NewCameraFOV = PlayerPawn->DefaultCameraFieldOfView; // Needs to be replaced with game settings value

		if (bCheckCameraVolumes)
		{
			// Get active camera volume according to overlapping and priority
			CamVolCurrent = nullptr;
			int8 MaxPriorityIndex = -100; // this is limited (-100, 100) in ACameraVolumeActor.Priority

			if (PlayerPawn->OverlappingCameraVolumes.Num() > 0)
			{// Try to get overlapping camera volumes stored in pawn
				for (ACameraVolumeActor* CamVol : PlayerPawn->OverlappingCameraVolumes)
				{
					if (CamVol)
					{
						CheckAndSortCameraVolumes(CamVol, PlayerPawnLocation, MaxPriorityIndex);
					}
				}
			}
			else
			{// Try to get camera volumes from actors overlapping pawn
				OverlappingActors.Empty();
				PlayerPawn->GetOverlappingActors(OverlappingActors, ACameraVolumeActor::StaticClass());
				if (OverlappingActors.Num() > 0)
				{
					for (AActor* Actor : OverlappingActors)
					{
						ACameraVolumeActor* CamVol = Cast<ACameraVolumeActor>(Actor);
						if (CamVol)
						{
							CheckAndSortCameraVolumes(CamVol, PlayerPawnLocation, MaxPriorityIndex);
						}
					}
				}
				else
					bCheckCameraVolumes = false;
				// There is no camera volumes overlapping character at this time,
				// so we don't need this check until player pawn overlap some camera volume.
			}
		}

		if (CamVolCurrent)
		{ // We have passed into camera volume
			// Detect passed side for transition
			if (CamVolCurrent != CamVolPrevious)
			{
				// Get passed side by distance
				PassedSide = GetNearestVolumeSide(CamVolCurrent, PlayerPawnLocation, AllowedDist);
				if (PassedSide == ESide::ES_Unknown)
				{
					// Maybe we has changed to volume with lower priority?
					if (CamVolPrevious && CamVolCurrent->Priority < CamVolPrevious->Priority)
					{
						// Use settings of side we has passed from
						PassedSide = GetNearestVolumeSide(CamVolPrevious, PlayerPawnLocation, AllowedDist);
					}
				}

				FSideInfo PassedSideInfo = CamVolCurrent->GetSideInfo(PassedSide);
				if (PassedSideInfo.SideTransitionType == ESideTransitionType::ESTT_Smooth)
				{
					bNeedsSmoothTransition = true;
					SmoothTransitionAlpha = 0.f;
				}
				else
					bNeedsCutTransition = true;
			}

			// Check is FOV overriden
			if (CamVolCurrent->bOverrideCameraFieldOfView)
			{
				NewCameraFOV = CamVolCurrent->CameraFieldOfView;
			}
			float PlayerCamFOVTangens = FMath::Tan((PI / (180.f)) * (NewCameraFOV * 0.5f));
			float PlayerCamAspectRatio = PlayerPawn->GetCameraComponent()->AspectRatio;

			// Calculate new location and rotation
			if (CamVolCurrent->bFixedCamera)
			{
				NewCameraLocation = CamVolCurrent->GetActorLocation() + CamVolCurrent->FixedCameraLocation;
				if (CamVolCurrent->bFocalPointIsPlayer)
				{
					NewCameraRotation = FRotationMatrix::MakeFromX(PlayerPawn->GetActorLocation() - CamVolCurrent->GetActorLocation() - CamVolCurrent->FixedCameraLocation).Rotator();
				}
				else
				{
					NewCameraRotation = CamVolCurrent->FixedCameraRotation;
				}
			}
			else
			{
				FVector DeltaExtent = FVector::ZeroVector;
				// Side-scroller
				DeltaExtent.Y = FMath::Abs(PlayerCamFOVTangens * CamVolCurrent->CamVolWorldMaxCorrected.X);
				DeltaExtent = FVector(0.f, DeltaExtent.Y, DeltaExtent.Y / PlayerCamAspectRatio);
				// Top-down
				//DeltaExtent.Y = FMath::Abs(PlayerCamFOVTangens * CamVolCurrent->CamVolWorldMaxCorrected.Z);
				//DeltaExtent = FVector(DeltaExtent.Y / PlayerCamAspectRatio, DeltaExtent.Y, 0.f);
				FVector NewCamVolExtentCorrected = CamVolCurrent->CamVolExtentCorrected + DeltaExtent;
				FVector NewCamVolWorldMinCorrected = CamVolCurrent->CamVolWorldMinCorrected - DeltaExtent;
				FVector NewCamVolWorldMaxCorrected = CamVolCurrent->CamVolWorldMaxCorrected + DeltaExtent;

				// Calculate camera offset
				float NewCameraOffset;
				NewCameraOffset = NewCameraLocation.X; // Side-scroller
				//NewCameraOffset = NewCameraLocation.Z; // Top-down
				if (CamVolCurrent->bOverrideCameraOffset)
					NewCameraOffset = CamVolCurrent->CameraOffset;

				if (CamVolCurrent->CamVolAspectRatio >= PlayerCamAspectRatio)
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

				// Calculate screen world extent at depth (NewCameraOffset)
				FVector ScreenExtent = FVector::ZeroVector;
				ScreenExtent.Y = FMath::Abs(PlayerCamFOVTangens * NewCameraOffset);
				// Side-scroller
				ScreenExtent = FVector(0.f, ScreenExtent.Y, ScreenExtent.Y / PlayerCamAspectRatio);
				// Top-down
				//ScreenExtent = FVector(ScreenExtent.Y / PlayerCamAspectRatio, ScreenExtent.Y, 0.f);
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

			// Calculate camera transition and lag
			CalcTransitionAndLag(DeltaTime);
		}
		else
		{ // We have passed from camera volume to void
			if (CamVolPrevious)
			{
				// Use settings of side we has passed from
				PassedSide = GetNearestVolumeSide(CamVolPrevious, PlayerPawnLocation, AllowedDist);
			}

			// Calculate camera transition and lag
			CalcTransitionAndLag(DeltaTime);
		}

		CamVolPrevious = CamVolCurrent;

		PlayerPawn->UpdateCamera(NewCameraLocation, NewCameraRotation, NewCameraFOV);
	}
}

ESide ACamVolPlayerCameraManager::GetNearestVolumeSide(ACameraVolumeActor* CamVol, FVector PlayerPawnLocation, float Distance)
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

void ACamVolPlayerCameraManager::CheckAndSortCameraVolumes(ACameraVolumeActor* CamVol, FVector PlayerPawnLocation, int8 MaxPriorityIndex)
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

void ACamVolPlayerCameraManager::CalcTransitionAndLag(float DeltaTime)
{
	if (bNeedsSmoothTransition)
	{
		if (SmoothTransitionAlpha <= CamVolCurrent->CameraSmoothTransitionTime)
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