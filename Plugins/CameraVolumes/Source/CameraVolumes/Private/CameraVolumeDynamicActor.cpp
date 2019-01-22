
#include "CameraVolumeDynamicActor.h"

ACameraVolumeDynamicActor::ACameraVolumeDynamicActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Default root
	DefaultSceneRoot->Mobility = EComponentMobility::Movable;
	bActive = false;
}

void ACameraVolumeDynamicActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bActive)
		UpdateVolumeExtents();
}

void ACameraVolumeDynamicActor::SetActive(bool bNewActive)
{
	bActive = bNewActive;
}