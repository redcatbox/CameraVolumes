//Dmitriy Barannik aka redbox, 2019

#include "CameraVolumesFunctionLibrary.h"

ACameraVolumeActor* UCameraVolumesFunctionLibrary::GetCurrentCameraVolume(TArray<ACameraVolumeActor*> CameraVolumes, FVector& PlayerPawnLocation)
{
	ACameraVolumeActor* Result = nullptr;
	int8 MaxPriorityIndex = -101; // this is clamped (-100, 100) in ACameraVolumeActor->Priority
	bool Condition = false;

	for (ACameraVolumeActor* CameraVolume : CameraVolumes)
	{
		if (CameraVolume)
		{
			if (CameraVolume->bUse6DOFVolume)
			{
				Condition = CameraVolume->CamVolWorldMin.X < PlayerPawnLocation.X
					&& PlayerPawnLocation.X < CameraVolume->CamVolWorldMax.X
					&& CameraVolume->CamVolWorldMin.Y < PlayerPawnLocation.Y
					&& PlayerPawnLocation.Y < CameraVolume->CamVolWorldMax.Y
					&& CameraVolume->CamVolWorldMin.Z < PlayerPawnLocation.Z
					&& PlayerPawnLocation.Z < CameraVolume->CamVolWorldMax.Z;
			}
			else
			{
				//Side-scroller
				Condition = CameraVolume->CamVolWorldMin.X < PlayerPawnLocation.X
					&& PlayerPawnLocation.X < CameraVolume->CamVolWorldMax.X
					&& CameraVolume->CamVolWorldMin.Z < PlayerPawnLocation.Z
					&& PlayerPawnLocation.Z < CameraVolume->CamVolWorldMax.Z;
				//Top-down
				//Condition = CameraVolume->CamVolWorldMin.X < PlayerPawnLocation.X
				//	&& PlayerPawnLocation.X < CameraVolume->CamVolWorldMax.X
				//	&& CameraVolume->CamVolWorldMin.Y < PlayerPawnLocation.Y
				//	&& PlayerPawnLocation.Y < CameraVolume->CamVolWorldMax.Y;
			}

			if (Condition && (CameraVolume->Priority > MaxPriorityIndex))
			{
				MaxPriorityIndex = CameraVolume->Priority;
				Result = CameraVolume;
			}
		}
	}

	return Result;
}

bool UCameraVolumesFunctionLibrary::CompareSidesPairs(ESide SideA, ESide SideB, bool b6DOFVolume)
{
	if (b6DOFVolume)
		if ((SideA == ESide::ES_Front && SideB == ESide::ES_Back)
			|| (SideA == ESide::ES_Back && SideB == ESide::ES_Front)
			|| (SideA == ESide::ES_Right && SideB == ESide::ES_Left)
			|| (SideA == ESide::ES_Left && SideB == ESide::ES_Right)
			|| (SideA == ESide::ES_Top && SideB == ESide::ES_Bottom)
			|| (SideA == ESide::ES_Bottom && SideB == ESide::ES_Top))
			return true;
		else
			//Side-scroller
			if ((SideA == ESide::ES_Right && SideB == ESide::ES_Left)
				|| (SideA == ESide::ES_Left && SideB == ESide::ES_Right)
				|| (SideA == ESide::ES_Top && SideB == ESide::ES_Bottom)
				|| (SideA == ESide::ES_Bottom && SideB == ESide::ES_Top))
			//Top-down
			//if ((SideA == ESide::ES_Front && SideB == ESide::ES_Back)
			//	|| (SideA == ESide::ES_Back && SideB == ESide::ES_Front)
			//	|| (SideA == ESide::ES_Right && SideB == ESide::ES_Left)
			//	|| (SideA == ESide::ES_Left && SideB == ESide::ES_Right))
					return true;

	return false;
}

FQuat UCameraVolumesFunctionLibrary::CalculateCameraRotation(FVector& CameraLocation, FVector& CameraFocalPoint, float CameraRoll)
{
	FQuat CameraRotation = (FRotationMatrix::MakeFromX(CameraFocalPoint - CameraLocation)).ToQuat();
	CameraRotation = FQuat(CameraRotation.GetAxisX(), FMath::DegreesToRadians(CameraRoll)) * CameraRotation;
	return CameraRotation;
}

FQuat UCameraVolumesFunctionLibrary::CalculateCameraRotationToCharacter(FVector& CameraLocation, FVector& CameraFocalPoint, float CameraRoll, FVector PlayerPawnLocation, FVector CameraVolumeLocation)
{
	FQuat CameraRotation = (FRotationMatrix::MakeFromX(PlayerPawnLocation - CameraVolumeLocation -  CameraLocation + CameraFocalPoint)).ToQuat();
	CameraRotation = FQuat(CameraRotation.GetAxisX(), FMath::DegreesToRadians(CameraRoll)) * CameraRotation;
	return CameraRotation;
}