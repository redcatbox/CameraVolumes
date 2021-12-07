// redbox, 2021

#include "CameraVolumesCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "CameraVolumesFunctionLibrary.h"
#include "CameraVolumesCameraManager.h"

ACameraVolumesCharacter::ACameraVolumesCharacter()
{
	// Create a camera
	CameraComponent = CreateDefaultSubobject<UCameraVolumesCameraComponent>(TEXT("Camera"));
	CameraComponent->bUsePawnControlRotation = false; // We don't want the controller rotating the camera

	// Configure character movement
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ACameraVolumesCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Bind overlap events
	GetCollisionPrimitiveComponent()->OnComponentBeginOverlap.AddDynamic(this, &ACameraVolumesCharacter::OnCollisionPrimitiveComponentBeginOverlap);
	GetCollisionPrimitiveComponent()->OnComponentEndOverlap.AddDynamic(this, &ACameraVolumesCharacter::OnCollisionPrimitiveComponentEndOverlap);
}

void ACameraVolumesCharacter::OnCollisionPrimitiveComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ACameraVolumeActor* CameraVolume = Cast<ACameraVolumeActor>(OtherActor))
	{
		if (GetCameraComponent()->OverlappingCameraVolumes.Num() == 0)
		{
			// Update camera volumes check condition in PlayerCameraManager
			if (APlayerController* PC = Cast<APlayerController>(GetController()))
			{
				if (ACameraVolumesCameraManager* CameraVolumePCM = Cast<ACameraVolumesCameraManager>(PC->PlayerCameraManager))
				{
					CameraVolumePCM->SetProcessCameraVolumes(true);
				}
			}
		}

		GetCameraComponent()->OverlappingCameraVolumes.Emplace(CameraVolume);
	}
}

void ACameraVolumesCharacter::OnCollisionPrimitiveComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ACameraVolumeActor* CameraVolume = Cast<ACameraVolumeActor>(OtherActor))
	{
		GetCameraComponent()->OverlappingCameraVolumes.Remove(CameraVolume);
	}
}

UCameraVolumesCameraComponent* ACameraVolumesCharacter::GetCameraComponent() const
{
	return CameraComponent;
}

UPrimitiveComponent* ACameraVolumesCharacter::GetCollisionPrimitiveComponent() const
{
	return GetCapsuleComponent();
}
