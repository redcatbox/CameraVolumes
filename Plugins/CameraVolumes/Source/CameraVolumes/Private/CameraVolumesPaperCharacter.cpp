// redbox, 2021

#include "CameraVolumesPaperCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "CameraVolumesFunctionLibrary.h"
#include "CameraVolumesCameraManager.h"

ACameraVolumesPaperCharacter::ACameraVolumesPaperCharacter()
{
	// Create a camera
	CameraComponent = CreateDefaultSubobject<UCameraVolumesCameraComponent>(TEXT("Camera"));
	CameraComponent->bUsePawnControlRotation = false; // We don't want the controller rotating the camera

	// Configure character movement
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void ACameraVolumesPaperCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Bind overlap events
	GetCollisionPrimitiveComponent()->OnComponentBeginOverlap.AddDynamic(this, &ACameraVolumesPaperCharacter::OnCollisionPrimitiveComponentBeginOverlap);
	GetCollisionPrimitiveComponent()->OnComponentEndOverlap.AddDynamic(this, &ACameraVolumesPaperCharacter::OnCollisionPrimitiveComponentEndOverlap);
}

void ACameraVolumesPaperCharacter::OnCollisionPrimitiveComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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
					CameraVolumePCM->SetCheckCameraVolumes(true);
				}
			}
		}

		GetCameraComponent()->OverlappingCameraVolumes.AddUnique(CameraVolume);
	}
}

void ACameraVolumesPaperCharacter::OnCollisionPrimitiveComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ACameraVolumeActor* CameraVolume = Cast<ACameraVolumeActor>(OtherActor))
	{
		GetCameraComponent()->OverlappingCameraVolumes.Remove(CameraVolume);
	}
}

UCameraVolumesCameraComponent* ACameraVolumesPaperCharacter::GetCameraComponent() const
{
	return CameraComponent;
}

UPrimitiveComponent* ACameraVolumesPaperCharacter::GetCollisionPrimitiveComponent() const
{
	return GetCapsuleComponent();
}
