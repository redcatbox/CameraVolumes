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
			FVector PlayerPawnLocationTransformed = CameraVolume->GetActorTransform().InverseTransformPositionNoScale(PlayerPawnLocation);

			if (CameraVolume->bUse6DOFVolume)
			{
				Condition = -CameraVolume->VolumeExtent.X < PlayerPawnLocationTransformed.X
					&& PlayerPawnLocationTransformed.X <= CameraVolume->VolumeExtent.X
					&& -CameraVolume->VolumeExtent.Y < PlayerPawnLocationTransformed.Y
					&& PlayerPawnLocationTransformed.Y <= CameraVolume->VolumeExtent.Y
					&& -CameraVolume->VolumeExtent.Z < PlayerPawnLocationTransformed.Z
					&& PlayerPawnLocationTransformed.Z <= CameraVolume->VolumeExtent.Z;
			}
			else
			{
				Condition = -CameraVolume->VolumeExtent.X < PlayerPawnLocationTransformed.X
					&& PlayerPawnLocationTransformed.X <= CameraVolume->VolumeExtent.X
					&& -CameraVolume->VolumeExtent.Z < PlayerPawnLocationTransformed.Z
					&& PlayerPawnLocationTransformed.Z <= CameraVolume->VolumeExtent.Z;
			}
		}

		if (Condition && (CameraVolume->Priority > MaxPriorityIndex))
		{
			MaxPriorityIndex = CameraVolume->Priority;
			Result = CameraVolume;
		}
	}

	return Result;
}

bool UCameraVolumesFunctionLibrary::CompareSidesPairs(ESide SideA, ESide SideB, bool b6DOFVolume)
{
	if (b6DOFVolume)
	{
		if ((SideA == ESide::ES_Front && SideB == ESide::ES_Back)
			|| (SideA == ESide::ES_Back && SideB == ESide::ES_Front)
			|| (SideA == ESide::ES_Right && SideB == ESide::ES_Left)
			|| (SideA == ESide::ES_Left && SideB == ESide::ES_Right)
			|| (SideA == ESide::ES_Top && SideB == ESide::ES_Bottom)
			|| (SideA == ESide::ES_Bottom && SideB == ESide::ES_Top))
		{
			return true;
		}
	}
	else
	{
		if ((SideA == ESide::ES_Right && SideB == ESide::ES_Left)
			|| (SideA == ESide::ES_Left && SideB == ESide::ES_Right)
			|| (SideA == ESide::ES_Top && SideB == ESide::ES_Bottom)
			|| (SideA == ESide::ES_Bottom && SideB == ESide::ES_Top))
		{
			return true;
		}
	}

	return false;
}

FQuat UCameraVolumesFunctionLibrary::CalculateCameraRotation(FVector& CameraLocation, FVector& CameraFocalPoint, float CameraRoll)
{
	FQuat CameraRotation = FRotationMatrix::MakeFromX(CameraFocalPoint - CameraLocation).ToQuat();
	CameraRotation = FQuat(CameraRotation.GetAxisX(), FMath::DegreesToRadians(CameraRoll)) * CameraRotation;
	return CameraRotation;
}