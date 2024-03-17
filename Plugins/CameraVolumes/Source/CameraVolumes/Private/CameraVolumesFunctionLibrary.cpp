// redbox, 2024

#include "CameraVolumesFunctionLibrary.h"
#include "CameraVolumesTypes.h"
#include "CameraVolumeActor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CameraVolumesFunctionLibrary)

const ACameraVolumeActor* UCameraVolumesFunctionLibrary::GetCurrentCameraVolume(const TSet<ACameraVolumeActor*>& InCameraVolumes, const FVector& InPlayerPawnLocation)
{
	const ACameraVolumeActor* Result = nullptr;
	int32 MaxPriority = -101;
	bool bInside = false;

	for (const ACameraVolumeActor* CameraVolume : InCameraVolumes)
	{
		if (CameraVolume)
		{
			FVector LocalPlayerPawnLocation = CameraVolume->GetActorTransform().InverseTransformPositionNoScale(InPlayerPawnLocation);

			if (CameraVolume->bUse6DOFVolume)
			{
				bInside = -CameraVolume->VolumeExtent.X <= LocalPlayerPawnLocation.X
					&& LocalPlayerPawnLocation.X <= CameraVolume->VolumeExtent.X
					&& -CameraVolume->VolumeExtent.Y <= LocalPlayerPawnLocation.Y
					&& LocalPlayerPawnLocation.Y <= CameraVolume->VolumeExtent.Y
					&& -CameraVolume->VolumeExtent.Z <= LocalPlayerPawnLocation.Z
					&& LocalPlayerPawnLocation.Z <= CameraVolume->VolumeExtent.Z;
			}
			else
			{
				bInside = -CameraVolume->VolumeExtent.X <= LocalPlayerPawnLocation.X
					&& LocalPlayerPawnLocation.X <= CameraVolume->VolumeExtent.X
					&& -CameraVolume->VolumeExtent.Z <= LocalPlayerPawnLocation.Z
					&& LocalPlayerPawnLocation.Z <= CameraVolume->VolumeExtent.Z;
			}
		}

		if (bInside
			&& CameraVolume->Priority > MaxPriority)
		{
			MaxPriority = CameraVolume->Priority;
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

FQuat UCameraVolumesFunctionLibrary::CalculateCameraRotation(const FVector& InCameraLocation, const FVector& InCameraFocalPoint, float InCameraRoll)
{
	FQuat CameraRotation = FRotationMatrix::MakeFromX(InCameraFocalPoint - InCameraLocation).ToQuat();
	CameraRotation = FQuat(CameraRotation.GetAxisX(), FMath::DegreesToRadians(InCameraRoll)) * CameraRotation;
	return CameraRotation;
}
