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

	/** Get current camera volume from array of volumes according to 4 sides, coordinates and priority */
	UFUNCTION(BlueprintCallable, Meta = (Category = "CameraVolumes"))
		static ACameraVolumeActor* GetCurrentCameraVolume2D(TArray<ACameraVolumeActor*> CameraVolumes, FVector& PlayerPawnLocation);

	/** Compare sides is them are in pair Front/Back, Right/Left, Top/Bottom */
	UFUNCTION(BlueprintCallable, Meta = (Category = "CameraVolumes"))
		static bool CompareSidesPairs(ESide SideA, ESide SideB);

	/** Compare sides is them are in pair Right/Left, Top/Bottom */
	UFUNCTION(BlueprintCallable, Meta = (Category = "CameraVolumes"))
		static bool CompareSidesPairs2D(ESide SideA, ESide SideB);
};
