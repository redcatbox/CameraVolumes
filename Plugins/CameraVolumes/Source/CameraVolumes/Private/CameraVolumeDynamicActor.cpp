//Dmitriy Barannik aka redbox, 2019

#include "CameraVolumeDynamicActor.h"

ACameraVolumeDynamicActor::ACameraVolumeDynamicActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Default root
	DefaultSceneRoot->Mobility = EComponentMobility::Movable;
	bActive = false;
<<<<<<< HEAD
<<<<<<< HEAD
	OldVolumeLocation = FVector::ZeroVector;
}

void ACameraVolumeDynamicActor::UpdateVolume()
{
	Super::UpdateVolume();
	OldVolumeLocation = GetActorLocation();
=======

=======
>>>>>>> 072045c... changed coord system
	OldVolumeLocation = FVector::ZeroVector;
<<<<<<< HEAD
	NewVolumeLocation = FVector::ZeroVector;
>>>>>>> a8e85d6... fff
=======
>>>>>>> 14484cc... fixed dynamic volume update
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
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 14484cc... fixed dynamic volume update
	FVector LocationDelta = GetActorLocation() - OldVolumeLocation;
	OldVolumeLocation += LocationDelta;
	CamVolWorldMin += LocationDelta;
	CamVolWorldMax += LocationDelta;
<<<<<<< HEAD
=======
	OldVolumeLocation = NewVolumeLocation;
	NewVolumeLocation = GetActorLocation();
	FVector LocationDelta = NewVolumeLocation - OldVolumeLocation;
>>>>>>> a8e85d6... fff
=======
>>>>>>> 14484cc... fixed dynamic volume update
	CamVolWorldMinCorrected += LocationDelta;
	CamVolWorldMaxCorrected += LocationDelta;
}

void ACameraVolumeDynamicActor::SetActive(bool bNewActive)
{
	bActive = bNewActive;
}