
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
	/** Get current camera volume from array of volumes according to coordinates and priority */
	UFUNCTION(BlueprintCallable, Meta = (Category = "CameraVolumes"))
		static ACameraVolumeActor* GetCurrentCameraVolume(TArray<ACameraVolumeActor*> CameraVolumes, FVector& PlayerPawnLocation);

	/** Compare sides is them are in pair Front/Back, Right/Left, Top/Bottom */
	UFUNCTION(BlueprintCallable, Meta = (Category = "CameraVolumes"))
		static bool CompareSidesPairs(ESide SideA, ESide SideB);
};
