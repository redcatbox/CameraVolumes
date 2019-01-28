//Dmitriy Barannik aka redbox, 2019

#include "CameraVolumesFunctionLibrary.h"

<<<<<<< HEAD
<<<<<<< HEAD
=======
	/** Get current camera volume from array of volumes according to 6 sides, coordinates and priority */
>>>>>>> bc8bf29... in progress 2
=======
>>>>>>> b8d6390... refactoring to match paper2d integration
ACameraVolumeActor* UCameraVolumesFunctionLibrary::GetCurrentCameraVolume(TArray<ACameraVolumeActor*> CameraVolumes, FVector& PlayerPawnLocation)
{
	ACameraVolumeActor* Result = nullptr;
	int8 MaxPriorityIndex = -101; // this is clamped (-100, 100) in ACameraVolumeActor->Priority
	bool Condition = false;

	for (ACameraVolumeActor* CameraVolume : CameraVolumes)
	{
		if (CameraVolume)
		{
<<<<<<< HEAD
<<<<<<< HEAD
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
					//&& PlayerPawnLocation.X < CameraVolume->CamVolWorldMax.X
					//&& CameraVolume->CamVolWorldMin.Y < PlayerPawnLocation.Y
					//&& PlayerPawnLocation.Y < CameraVolume->CamVolWorldMax.Y;
			}

			if (Condition && (CameraVolume->Priority > MaxPriorityIndex))
			{
				MaxPriorityIndex = CameraVolume->Priority;
				Result = CameraVolume;
=======
			if (CameraVolume->CamVolWorldMin.X < PlayerPawnLocation.X &&
				PlayerPawnLocation.X < CameraVolume->CamVolWorldMax.X &&
				CameraVolume->CamVolWorldMin.Y < PlayerPawnLocation.Y &&
				PlayerPawnLocation.Y < CameraVolume->CamVolWorldMax.Y &&
				CameraVolume->CamVolWorldMin.Z < PlayerPawnLocation.Z &&
				PlayerPawnLocation.Z < CameraVolume->CamVolWorldMax.Z)
=======
			if (CameraVolume->bUse6DOFVolume)
>>>>>>> 072045c... changed coord system
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
					//&& PlayerPawnLocation.X < CameraVolume->CamVolWorldMax.X
					//&& CameraVolume->CamVolWorldMin.Y < PlayerPawnLocation.Y
					//&& PlayerPawnLocation.Y < CameraVolume->CamVolWorldMax.Y;
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

<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
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
>>>>>>> bc8bf29... in progress 2
			}
		}
	}

	return Result;
}

<<<<<<< HEAD
bool UCameraVolumesFunctionLibrary::CompareSidesPairs(ESide SideA, ESide SideB, bool b6DOFVolume)
=======
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

=======
>>>>>>> a8e85d6... fff
bool UCameraVolumesFunctionLibrary::CompareSidesPairs(ESide SideA, ESide SideB)
>>>>>>> b8d6390... refactoring to match paper2d integration
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

<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
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
=======
/** Compare sides is them are in pair Right/Left, Top/Bottom */
bool UCameraVolumesFunctionLibrary::CompareSidesPairs2D(ESide SideA, ESide SideB)
=======
bool UCameraVolumesFunctionLibrary::CompareSidesPairs2DYZ(ESide SideA, ESide SideB)
>>>>>>> b8d6390... refactoring to match paper2d integration
{
	if ((SideA == ESide::ES_Right && SideB == ESide::ES_Left)
=======
bool UCameraVolumesFunctionLibrary::CompareSidesPairs(ESide SideA, ESide SideB, bool b6SidesVolume)
{
	if (b6SidesVolume)
	if ((SideA == ESide::ES_Front && SideB == ESide::ES_Back)
		|| (SideA == ESide::ES_Back && SideB == ESide::ES_Front)
		|| (SideA == ESide::ES_Right && SideB == ESide::ES_Left)
>>>>>>> ec14146... added character interface
		|| (SideA == ESide::ES_Left && SideB == ESide::ES_Right)
		|| (SideA == ESide::ES_Top && SideB == ESide::ES_Bottom)
		|| (SideA == ESide::ES_Bottom && SideB == ESide::ES_Top))
		return true;
	else
		//Side-scroller
		if ((SideA == ESide::ES_Right && SideB == ESide::ES_Left)
=======
bool UCameraVolumesFunctionLibrary::CompareSidesPairs(ESide SideA, ESide SideB, bool b6DOFVolume)
{
	if (b6DOFVolume)
		if ((SideA == ESide::ES_Front && SideB == ESide::ES_Back)
			|| (SideA == ESide::ES_Back && SideB == ESide::ES_Front)
			|| (SideA == ESide::ES_Right && SideB == ESide::ES_Left)
>>>>>>> 072045c... changed coord system
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
<<<<<<< HEAD
>>>>>>> bc8bf29... in progress 2
=======
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

=======
>>>>>>> a8e85d6... fff
FQuat UCameraVolumesFunctionLibrary::CalculateCameraRotation(FVector& CameraLocation, FVector& CameraFocalPoint, float CameraRoll)
{
	FQuat CameraRotation = (FRotationMatrix::MakeFromX(CameraFocalPoint - CameraLocation)).ToQuat();
	CameraRotation = FQuat(CameraRotation.GetAxisX(), FMath::DegreesToRadians(CameraRoll)) * CameraRotation;
	return CameraRotation;
<<<<<<< HEAD
>>>>>>> b8d6390... refactoring to match paper2d integration
=======
}

FQuat UCameraVolumesFunctionLibrary::CalculateCameraRotationToCharacter(FVector& CameraLocation, FVector& CameraFocalPoint, float CameraRoll, FVector PlayerPawnLocation, FVector CameraVolumeLocation)
{
	FQuat CameraRotation = (FRotationMatrix::MakeFromX(PlayerPawnLocation - CameraVolumeLocation -  CameraLocation + CameraFocalPoint)).ToQuat();
	CameraRotation = FQuat(CameraRotation.GetAxisX(), FMath::DegreesToRadians(CameraRoll)) * CameraRotation;
	return CameraRotation;
>>>>>>> fde4429... wip
}