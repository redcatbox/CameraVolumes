//Dmitriy Barannik aka redbox, 2019

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CameraVolumeActor.h"
#include "CameraVolumesFunctionLibrary.generated.h"

UCLASS()
class CAMERAVOLUMES_API UCameraVolumesFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/** Get current camera volume from array of volumes according to 6 sides, coordinates and priority */
	UFUNCTION(BlueprintCallable, Meta = (Category = "CameraVolumes"))
		static ACameraVolumeActor* GetCurrentCameraVolume(TArray<ACameraVolumeActor*> CameraVolumes, FVector& PlayerPawnLocation);

<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
	/** Check is sides are in pair Front/Back, Right/Left, Top/Bottom */
	UFUNCTION(BlueprintCallable, Meta = (Category = "CameraVolumes"))
		static bool CompareSidesPairs(ESide SideA, ESide SideB, bool b6DOFVolume);

	/** Calculate camera rotation from camera position, focal point and roll */
	UFUNCTION(BlueprintCallable, Meta = (Category = "CameraVolumes"))
		static FQuat CalculateCameraRotation(FVector& CameraLocation, FVector& CameraFocalPoint, float CameraRoll);

	/** Calculate camera rotation from camera position, focal point, roll, player and camera volume positions */
	UFUNCTION(BlueprintCallable, Meta = (Category = "CameraVolumes"))
		static FQuat CalculateCameraRotationToCharacter(FVector& CameraLocation, FVector& CameraFocalPoint, float CameraRoll, FVector PlayerPawnLocation, FVector CameraVolumeLocation);

	
=======
	/** Get current camera volume from array of volumes according to 4 sides, coordinates and priority */
=======
	/** Get current camera volume from array of volumes according to 4 sides, YZ plane, coordinates and priority */
>>>>>>> b8d6390... refactoring to match paper2d integration
	UFUNCTION(BlueprintCallable, Meta = (Category = "CameraVolumes"))
		static ACameraVolumeActor* GetCurrentCameraVolume2DYZ(TArray<ACameraVolumeActor*> CameraVolumes, FVector& PlayerPawnLocation);

	/** Get current camera volume from array of volumes according to 4 sides, YX plane, coordinates and priority */
	UFUNCTION(BlueprintCallable, Meta = (Category = "CameraVolumes"))
		static ACameraVolumeActor* GetCurrentCameraVolume2DYX(TArray<ACameraVolumeActor*> CameraVolumes, FVector& PlayerPawnLocation);

=======
>>>>>>> a8e85d6... fff
	/** Check is sides are in pair Front/Back, Right/Left, Top/Bottom */
	UFUNCTION(BlueprintCallable, Meta = (Category = "CameraVolumes"))
		static bool CompareSidesPairs(ESide SideA, ESide SideB, bool b6DOFVolume);

	/** Calculate camera rotation from camera position, focal point and roll */
	UFUNCTION(BlueprintCallable, Meta = (Category = "CameraVolumes"))
<<<<<<< HEAD
		static bool CompareSidesPairs2D(ESide SideA, ESide SideB);
>>>>>>> bc8bf29... in progress 2
=======
		static FQuat CalculateCameraRotation(FVector& CameraLocation, FVector& CameraFocalPoint, float CameraRoll);
<<<<<<< HEAD
>>>>>>> b8d6390... refactoring to match paper2d integration
=======

	/** Calculate camera rotation from camera position, focal point, roll, player and camera volume positions */
	UFUNCTION(BlueprintCallable, Meta = (Category = "CameraVolumes"))
		static FQuat CalculateCameraRotationToCharacter(FVector& CameraLocation, FVector& CameraFocalPoint, float CameraRoll, FVector PlayerPawnLocation, FVector CameraVolumeLocation);

	
>>>>>>> fde4429... wip
};
