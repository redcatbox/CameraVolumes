// Dmitriy Barannik aka redbox, 2019
/**
* Player camera manager process camera calculations according to default camera parameters from player pawn or parameters from camera volumes.
* You can find information about basic algorithm in the end of file.
*/

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "CameraVolumeActor.h"
#include "CamVolCharacter.h"
#include "CamVolPlayerCameraManager.generated.h"

UCLASS()
class CAMVOL_API ACamVolPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	ACamVolPlayerCameraManager();
	virtual void UpdateCamera(float DeltaTime) override;

	/** This condition can be used for optimization purpose. You can disable it on loading/cutscenes/etc. */
	UPROPERTY()
		bool bCheckCameraVolumes;

	/** Get volume side nearest to player location */
	UFUNCTION()
		virtual ESide GetNearestVolumeSide(ACameraVolumeActor* CamVol, FVector& PlayerPawnLocation, float Distance);

	/** Set transition according to side info */
	UFUNCTION()
		virtual void SetTransitionBySideInfo(ACameraVolumeActor* CamVol, ESide Side);

	/** Check is character pivot inside of volume. */
	UFUNCTION()
		virtual void GetCurrentCameraVolume(ACameraVolumeActor* CamVol, FVector& PlayerPawnLocation, int8 MaxPriorityIndex);

	/** Calculate new camera parameters */
	UFUNCTION()
		virtual void CalcNewCameraParams(ACameraVolumeActor* CamVol);

	/** Calculate camera transition and lag */
	UFUNCTION()
		virtual void CalcTransitionAndLag(float DeltaTime);

protected:
	UPROPERTY()
		ACamVolCharacter* PlayerPawn;

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
		ESide PassedSide;

	UPROPERTY()
		bool bNeedsSmoothTransition;

	UPROPERTY()
		float SmoothTransitionAlpha;

	UPROPERTY()
		float SmoothTransitionTime;

	UPROPERTY()
		bool bNeedsCutTransition;
	
	const float AllowedDist = 25.f;
};

/** Basic algorithm
Player pawn enables camera volumes check when it's BeginOverlap triggered with camera volume actor.

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
	}

	if (array of volumes is empty)
	{
		current volume is not valid;
		disable camera volumes check; // until character overlaps new volume.
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
			// if (nearest side is unknown) // maybe we've been teleported? ok, unknown side have cut transition type.
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
			get transition type from side info;
			if (nearest side is unknown) // maybe we've passed not near any side?
			{
				if (previous volume is valid)
				{
					get nearest side for previous volume;
					// if (nearest side is unknown) // impossible situation
					get transition type from side info;
				}
			}

			use camera parameters from volume;
			calculate new camera transform according to transition and lag;
		}
	}
}
*/