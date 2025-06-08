// redbox, 2025

#include "CameraVolumeDynamicActor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CameraVolumeDynamicActor)

ACameraVolumeDynamicActor::ACameraVolumeDynamicActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Default root
	RootComponent->SetMobility(EComponentMobility::Movable);
}
