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
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		static ACameraVolumeActor* GetCurrentCameraVolume(TArray<ACameraVolumeActor*> CameraVolumes, FVector& PlayerPawnLocation);

	/** Check is sides are in pair Front/Back, Right/Left, Top/Bottom */
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		static bool CompareSidesPairs(ESide SideA, ESide SideB, bool b6DOFVolume);

	/** Calculate camera rotation from camera position, focal point and roll */
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		static FQuat CalculateCameraRotation(FVector& CameraLocation, FVector& CameraFocalPoint, float CameraRoll);
};