
/**
* Player camera manager process camera calculations according to default camera parameters from player pawn or parameters from camera volumes.
* You can find information about basic algorithm in the end of file.
*/

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "CameraVolumeDynamicActor.h"
#include "CameraVolumesFunctionLibrary.h"
#include "CameraVolumesCharacter.h"
#include "CameraVolumesCameraManager.generated.h"

UCLASS()
class CAMERAVOLUMES_API ACameraVolumesCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	ACameraVolumesCameraManager();
	virtual void UpdateCamera(float DeltaTime) override;

	/** This condition can be used for optimization purpose. You can disable it on loading/cutscenes/etc. */
	UPROPERTY()
		bool bCheckCameraVolumes;

	/** Set transition according to side info */
	UFUNCTION()
		virtual void SetTransitionBySideInfo(ACameraVolumeActor* CameraVolume, ESide Side);

	/** Calculate new camera parameters */
	UFUNCTION()
		virtual void CalcNewCameraParams(ACameraVolumeActor* CameraVolume, float DeltaTime);

protected:
	UPROPERTY()
		ACameraVolumesCharacter* PlayerPawn;

	UPROPERTY()
		FVector PlayerPawnLocation;

	UPROPERTY()
		TArray<AActor*> OverlappingActors;

	UPROPERTY()
		ACameraVolumeActor* CamVolCurrent;

	UPROPERTY()
		ACameraVolumeActor* CamVolPrevious;

	UPROPERTY()
		FVector OldCameraLocation;

	UPROPERTY()
		FVector NewCameraLocation;

	UPROPERTY()
		FRotator OldCameraRotation;

	UPROPERTY()
		FRotator NewCameraRotation;

	UPROPERTY()
		float OldCameraFOV;

	UPROPERTY()
		float NewCameraFOV;

	UPROPERTY()
		bool bNeedsSmoothTransition;

	UPROPERTY()
		float SmoothTransitionAlpha;

	UPROPERTY()
		float SmoothTransitionTime;

	UPROPERTY()
		bool bNeedsCutTransition;
};

/**	Basic algorithm
*	Player pawn enables camera volumes check when it's BeginOverlap triggered with camera volume actor.

if (camera volumes check is disabled)
{
	use camera parameters from pawn;
	calculate new camera transform according to transition and lag;
}
else
{
	get array of volumes stored in pawn;
	if (array of volumes is empty)
	{
		get array of volumes by GetOverlappingActors function;
		if (array of volumes is empty)
		{
			current volume is not valid;
			disable camera volumes check; // until character overlaps new volume
		}
	}
	else
	{
		process volumes array
		{
			if (player pivot is inside volume)
			{
				get volume with highest priority and store as current volume;
			}
			else
			{
				current volume is not valid;
			}
		}
	}

	if (current volume is not valid)
	{
		// maybe we've passed from volume into void?
		if (previous volume is valid)
		{
			get nearest side for previous volume;
			get transition type from side info;
		}

		use camera parameters from pawn;
		calculate new camera transform according to transition and lag;
	}
	else (current volume is valid)
	{
		if (current volume == previous volume)
		{
			use camera parameters from volume;
			calculate new camera transform according to transition and lag;
		}
		else (current volume != previous volume)
		{
			get nearest side for current volume;
			if (previous volume is valid)
			{
				get nearest side for previous volume;

				if (nearest side for current volume is in pair with nearest side for previous volume)
				{// we've passed to nearby volume
					get transition type from current volume nearest side info;
				}
				else
				{// we've passed to volume not from side
					if (current camera volume priority > previous camera volume priority )
						get transition type from current volume nearest side info;
					else
						get transition type from previous volume nearest side info;
				}
			}

			use camera parameters from volume;
			calculate new camera transform according to transition and lag;
		}
	}
}

*/