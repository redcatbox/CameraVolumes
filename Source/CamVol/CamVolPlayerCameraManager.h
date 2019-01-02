// Fill out your copyright notice in the Description page of Project Settings.

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
		virtual ESide GetNearestVolumeSide(ACameraVolumeActor* CamVol, FVector PlayerPawnLocation, float Distance);

	/** Check is character pivot inside of volume. */
	UFUNCTION()
		virtual void CheckAndSortCameraVolumes(ACameraVolumeActor* CamVol, FVector PlayerPawnLocation, int8 MaxPriorityIndex);

	/** Calculate camera transition and lag */
	UFUNCTION()
		virtual void CalcTransitionAndLag(float DeltaTime);

protected:
	UPROPERTY()
		ACamVolCharacter* PlayerPawn;

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
		bool bNeedsCutTransition;
	
	const float AllowedDist = 25.f;
};
