// redbox, 2024

#include "CameraVolumesCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "CameraVolumeActor.h"
#include "CameraVolumesCameraManager.h"
#include "CameraVolumesFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CameraVolumesCharacter)

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

ACameraVolumesCharacter::ACameraVolumesCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
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
	UPrimitiveComponent* CollisionPrimitiveComponent = GetCollisionPrimitiveComponent();
	if (IsValid(CollisionPrimitiveComponent))
	{
		CollisionPrimitiveComponent->OnComponentBeginOverlap.AddDynamic(this, &ACameraVolumesCharacter::OnCollisionPrimitiveComponentBeginOverlap);
		CollisionPrimitiveComponent->OnComponentEndOverlap.AddDynamic(this, &ACameraVolumesCharacter::OnCollisionPrimitiveComponentEndOverlap);
	}
}

void ACameraVolumesCharacter::OnCollisionPrimitiveComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACameraVolumeActor* CameraVolume = Cast<ACameraVolumeActor>(OtherActor);
	if (IsValid(CameraVolume))
	{
		if (IsValid(CameraComponent))
		{
			if (CameraComponent->OverlappingCameraVolumes.Num() == 0)
			{
				// Update camera volumes check condition in PlayerCameraManager
				APlayerController* PC = Cast<APlayerController>(GetController());
				if (IsValid(PC))
				{
					ACameraVolumesCameraManager* CameraVolumePCM = Cast<ACameraVolumesCameraManager>(PC->PlayerCameraManager);
					if (IsValid(CameraVolumePCM))
					{
						CameraVolumePCM->SetProcessCameraVolumes(true);
					}
				}
			}

			CameraComponent->OverlappingCameraVolumes.Emplace(CameraVolume);
		}
	}
}

void ACameraVolumesCharacter::OnCollisionPrimitiveComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACameraVolumeActor* CameraVolume = Cast<ACameraVolumeActor>(OtherActor);
	if (IsValid(CameraVolume))
	{
		if (IsValid(CameraComponent))
		{
			CameraComponent->OverlappingCameraVolumes.Remove(CameraVolume);
		}
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
