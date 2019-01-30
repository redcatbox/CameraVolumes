//Dmitriy Barannik aka redbox, 2019

#include "CameraVolumesFunctionLibrary.h"

ACameraVolumeActor* UCameraVolumesFunctionLibrary::GetCurrentCameraVolume(TArray<ACameraVolumeActor*> CameraVolumes, FVector& PlayerPawnLocation)
{
	ACameraVolumeActor* Result = nullptr;
	int8 MaxPriorityIndex = -100; // this is limited (-100, 100) in ACameraVolumeActor->Priority

	for (ACameraVolumeActor* CameraVolume : CameraVolumes)
	{
		if (CameraVolume)
		{
			if (CameraVolume->CamVolWorldMin.X < PlayerPawnLocation.X &&
				PlayerPawnLocation.X < CameraVolume->CamVolWorldMax.X &&
				CameraVolume->CamVolWorldMin.Y < PlayerPawnLocation.Y &&
				PlayerPawnLocation.Y < CameraVolume->CamVolWorldMax.Y &&
				CameraVolume->CamVolWorldMin.Z < PlayerPawnLocation.Z &&
				PlayerPawnLocation.Z < CameraVolume->CamVolWorldMax.Z)
			{
				if (CameraVolume->Priority > MaxPriorityIndex)
				{
					MaxPriorityIndex = CameraVolume->Priority;
					Result = CameraVolume;
				}
			}
		}
	}

	return Result;
}

ACameraVolumeActor* UCameraVolumesFunctionLibrary::GetCurrentCameraVolume2DYZ(TArray<ACameraVolumeActor*> CameraVolumes, FVector& PlayerPawnLocation)
{
	ACameraVolumeActor* Result = nullptr;
	int8 MaxPriorityIndex = -100; // this is limited (-100, 100) in ACameraVolumeActor->Priority

	for (ACameraVolumeActor* CameraVolume : CameraVolumes)
	{
		if (CameraVolume)
		{
			if (CameraVolume->CamVolWorldMin.Y < PlayerPawnLocation.Y &&
				PlayerPawnLocation.Y < CameraVolume->CamVolWorldMax.Y &&
				CameraVolume->CamVolWorldMin.Z < PlayerPawnLocation.Z &&
				PlayerPawnLocation.Z < CameraVolume->CamVolWorldMax.Z)
			{
				if (CameraVolume->Priority > MaxPriorityIndex)
				{
					MaxPriorityIndex = CameraVolume->Priority;
					Result = CameraVolume;
				}
			}
		}
	}

	return Result;
}

ACameraVolumeActor* UCameraVolumesFunctionLibrary::GetCurrentCameraVolume2DYX(TArray<ACameraVolumeActor*> CameraVolumes, FVector& PlayerPawnLocation)
{
	ACameraVolumeActor* Result = nullptr;
	int8 MaxPriorityIndex = -100; // this is limited (-100, 100) in ACameraVolumeActor->Priority

	for (ACameraVolumeActor* CameraVolume : CameraVolumes)
	{
		if (CameraVolume)
		{
			if (CameraVolume->CamVolWorldMin.X < PlayerPawnLocation.X &&
				PlayerPawnLocation.X < CameraVolume->CamVolWorldMax.X &&
				CameraVolume->CamVolWorldMin.Y < PlayerPawnLocation.Y &&
				PlayerPawnLocation.Y < CameraVolume->CamVolWorldMax.Y)
			{
				if (CameraVolume->Priority > MaxPriorityIndex)
				{
					MaxPriorityIndex = CameraVolume->Priority;
					Result = CameraVolume;
				}
			}
		}
	}

	return Result;
}

bool UCameraVolumesFunctionLibrary::CompareSidesPairs(ESide SideA, ESide SideB)
{
	if ((SideA == ESide::ES_Front && SideB == ESide::ES_Back)
		|| (SideA == ESide::ES_Back && SideB == ESide::ES_Front)
		|| (SideA == ESide::ES_Right && SideB == ESide::ES_Left)
		|| (SideA == ESide::ES_Left && SideB == ESide::ES_Right)
		|| (SideA == ESide::ES_Top && SideB == ESide::ES_Bottom)
		|| (SideA == ESide::ES_Bottom && SideB == ESide::ES_Top))
		return true;

	return false;
}

bool UCameraVolumesFunctionLibrary::CompareSidesPairs2DYZ(ESide SideA, ESide SideB)
{
	if ((SideA == ESide::ES_Right && SideB == ESide::ES_Left)
		|| (SideA == ESide::ES_Left && SideB == ESide::ES_Right)
		|| (SideA == ESide::ES_Top && SideB == ESide::ES_Bottom)
		|| (SideA == ESide::ES_Bottom && SideB == ESide::ES_Top))
		return true;

	return false;
}

bool UCameraVolumesFunctionLibrary::CompareSidesPairs2DYX(ESide SideA, ESide SideB)
{
	if ((SideA == ESide::ES_Front && SideB == ESide::ES_Back)
		|| (SideA == ESide::ES_Back && SideB == ESide::ES_Front)
		|| (SideA == ESide::ES_Right && SideB == ESide::ES_Left)
		|| (SideA == ESide::ES_Left && SideB == ESide::ES_Right))
		return true;

	return false;
}

FQuat UCameraVolumesFunctionLibrary::CalculateCameraRotation(FVector& CameraLocation, FVector& CameraFocalPoint, float CameraRoll)
{
	FQuat CameraRotation = FRotationMatrix::MakeFromX(CameraFocalPoint - CameraLocation).ToQuat();
	CameraRotation = FQuat(CameraRotation.GetForwardVector(), FMath::DegreesToRadians(CameraRoll)) * CameraRotation;
	return CameraRotation;
}