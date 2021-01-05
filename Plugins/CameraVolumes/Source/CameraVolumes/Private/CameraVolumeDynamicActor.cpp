//redbox, 2021

#include "CameraVolumeDynamicActor.h"

ACameraVolumeDynamicActor::ACameraVolumeDynamicActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Default root
	DefaultSceneRoot->Mobility = EComponentMobility::Movable;
}
