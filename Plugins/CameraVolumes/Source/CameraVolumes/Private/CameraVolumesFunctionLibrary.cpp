//Dmitriy Barannik aka redbox, 2019

#include "CameraVolumesFunctionLibrary.h"

/** Get current camera volume from array of volumes according to coordinates and priority */
ACameraVolumeActor* UCameraVolumesFunctionLibrary::GetCurrentCameraVolume(TArray<ACameraVolumeActor*> CameraVolumes, FVector& PlayerPawnLocation)
{
	ACameraVolumeActor* Result = nullptr;
	int8 MaxPriorityIndex = -100; // this is limited (-100, 100) in ACameraVolumeActor->Priority

	for (ACameraVolumeActor* CamVol : CameraVolumes)
	{
		if (CamVol)
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
					Result = CamVol;
				}
			}
		}
	}

	return Result;
}

/** Compare sides is them are in pair Front/Back, Right/Left, Top/Bottom */
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