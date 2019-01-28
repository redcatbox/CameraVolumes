//Dmitriy Barannik aka redbox, 2019

#include "CameraVolumesCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "CameraVolumesFunctionLibrary.h"
#include "CameraVolumesCameraManager.h"

ACameraVolumesCharacter::ACameraVolumesCharacter()
{
	// Bind overlap events
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ACameraVolumesCharacter::OnCapsuleComponentBeginOverlapDelegate);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &ACameraVolumesCharacter::OnCapsuleComponentEndOverlapDelegate);

	// Create a camera
	CameraComponent = CreateDefaultSubobject<UCameraVolumesCameraComponent>(TEXT("Camera"));
	CameraComponent->bUsePawnControlRotation = false; // We don't want the controller rotating the camera

	// Configure character movement
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
<<<<<<< HEAD
<<<<<<< HEAD
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
=======

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Face in the direction we are moving...
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f); // ...at this rotation rate
>>>>>>> b8d6390... refactoring to match paper2d integration
=======
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
>>>>>>> 072045c... changed coord system
}

void ACameraVolumesCharacter::OnCapsuleComponentBeginOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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
				CameraVolumePCM->SetCheckCameraVolumes(true);
		}
	}
}

void ACameraVolumesCharacter::OnCapsuleComponentEndOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACameraVolumeActor* CameraVolume = Cast<ACameraVolumeActor>(OtherActor);
	if (CameraVolume)
<<<<<<< HEAD
<<<<<<< HEAD
		GetCameraComponent()->OverlappingCameraVolumes.Remove(CameraVolume);
=======
		OverlappingCameraVolumes.Remove(CameraVolume);
=======
		GetCameraComponent()->OverlappingCameraVolumes.Remove(CameraVolume);
>>>>>>> 6384029... fixed paper 2d character
}
<<<<<<< HEAD

//Update with changed property
#if WITH_EDITOR
void ACameraVolumesCharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == TEXT("DefaultCameraLocation") || TEXT("DefaultCameraFocalPoint") || TEXT("DefaultCameraFieldOfView"))
		UpdateCameraComponent();
>>>>>>> 55a984a... in progress
}

UCameraVolumesCameraComponent* ACameraVolumesCharacter::GetCameraComponent() const
{
	return CameraComponent;
}
<<<<<<< HEAD
=======
>>>>>>> b8d6390... refactoring to match paper2d integration
=======

UCameraVolumesCameraComponent* ACameraVolumesCharacter::GetCameraComponent() const
{
	return CameraComponent;
}
>>>>>>> ec14146... added character interface
