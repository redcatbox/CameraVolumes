//Dmitriy Barannik aka redbox, 2019

#include "CameraVolumesPaperCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "CameraVolumesFunctionLibrary.h"
#include "CameraVolumesCameraManager.h"

ACameraVolumesPaperCharacter::ACameraVolumesPaperCharacter()
{
	// Bind overlap events
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ACameraVolumesPaperCharacter::OnCapsuleComponentBeginOverlapDelegate);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &ACameraVolumesPaperCharacter::OnCapsuleComponentEndOverlapDelegate);

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
}

void ACameraVolumesPaperCharacter::OnCapsuleComponentBeginOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACameraVolumeActor* CameraVolume = Cast<ACameraVolumeActor>(OtherActor);
	if (CameraVolume)
	{
		GetCameraComponent()->OverlappingCameraVolumes.AddUnique(CameraVolume);

		// Update camera volumes check condition in PlayerCameraManager
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			ACameraVolumesCameraManager* CameraVolumePCM = Cast<ACameraVolumesCameraManager>(PC->PlayerCameraManager);
			if (CameraVolumePCM)
			{
				CameraVolumePCM->SetCheckCameraVolumes(true);
			}
		}
	}
}

void ACameraVolumesPaperCharacter::OnCapsuleComponentEndOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACameraVolumeActor* CameraVolume = Cast<ACameraVolumeActor>(OtherActor);
	if (CameraVolume)
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