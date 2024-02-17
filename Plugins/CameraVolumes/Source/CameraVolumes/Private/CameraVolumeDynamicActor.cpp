// redbox, 2024

#include "CameraVolumeDynamicActor.h"

ACameraVolumeDynamicActor::ACameraVolumeDynamicActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Default root
	RootComponent->SetMobility(EComponentMobility::Movable);
}
