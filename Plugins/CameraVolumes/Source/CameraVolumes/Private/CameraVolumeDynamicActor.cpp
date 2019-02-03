//Dmitriy Barannik aka redbox, 2019

#include "CameraVolumeDynamicActor.h"

ACameraVolumeDynamicActor::ACameraVolumeDynamicActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Default root
	DefaultSceneRoot->Mobility = EComponentMobility::Movable;
	bActive = false;
	OldVolumeLocation = FVector::ZeroVector;
}

void ACameraVolumeDynamicActor::UpdateVolume()
{
	Super::UpdateVolume();
	OldVolumeLocation = GetActorLocation();
}

void ACameraVolumeDynamicActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bActive)
		UpdateVolumeExtents();
}

void ACameraVolumeDynamicActor::UpdateVolumeExtents()
{
	FVector LocationDelta = GetActorLocation() - OldVolumeLocation;
	OldVolumeLocation += LocationDelta;
	CamVolWorldMin += LocationDelta;
	CamVolWorldMax += LocationDelta;
	CamVolWorldMinCorrected += LocationDelta;
	CamVolWorldMaxCorrected += LocationDelta;
}

void ACameraVolumeDynamicActor::SetActive(bool bNewActive)
{
	bActive = bNewActive;
}