//redbox, 2019

#include "CameraVolumeDynamicActor.h"

ACameraVolumeDynamicActor::ACameraVolumeDynamicActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Default root
	DefaultSceneRoot->Mobility = EComponentMobility::Movable;
}