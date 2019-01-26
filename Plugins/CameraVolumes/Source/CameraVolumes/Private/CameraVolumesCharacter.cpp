//Dmitriy Barannik aka redbox, 2019

#include "CameraVolumesCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CameraVolumesCameraManager.h"

ACameraVolumesCharacter::ACameraVolumesCharacter()
{
	// Bind overlap events
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ACameraVolumesCharacter::OnCapsuleComponentBeginOverlapDelegate);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &ACameraVolumesCharacter::OnCapsuleComponentEndOverlapDelegate);

	// Camera defaults
	DefaultCameraLocation = FVector(1000.f, 0.f, 0.f);
	DefaultCameraFocalPoint = FVector::ZeroVector;
	DefaultCameraFieldOfView = 90.f;

	// Camera lag
	bEnableCameraLocationLag = false;
	CameraLocationLagSpeed = 10.0f;
	bEnableCameraRotationLag = false;
	CameraRotationLagSpeed = 10.0f;
	bEnableCameraFOVInterpolation = false;
	CameraFOVInterpolationSpeed = 10.f;

	// Create a camera
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->bUsePawnControlRotation = false; // We don't want the controller rotating the camera
	UpdateCameraComponent();

	// Don't rotate when the controller rotates.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Face in the direction we are moving...
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f); // ...at this rotation rate
}

void ACameraVolumesCharacter::UpdateCamera(FVector& CameraLocation, FQuat& CameraRotation, float CameraFOV)
{
	if (CameraComponent)
	{
		CameraComponent->SetWorldLocation(CameraLocation);
		CameraComponent->SetWorldRotation(CameraRotation);
		CameraComponent->SetFieldOfView(CameraFOV);
	}
}

void ACameraVolumesCharacter::OnCapsuleComponentBeginOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACameraVolumeActor* CameraVolume = Cast<ACameraVolumeActor>(OtherActor);
	if (CameraVolume)
	{
		OverlappingCameraVolumes.AddUnique(CameraVolume);

		// Update camera volumes check condition in PlayerCameraManager
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			ACameraVolumesCameraManager* CameraVolumePCM = Cast<ACameraVolumesCameraManager>(PC->PlayerCameraManager);
			if (CameraVolumePCM)
				CameraVolumePCM->bCheckCameraVolumes = true;
		}
	}
}

void ACameraVolumesCharacter::OnCapsuleComponentEndOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACameraVolumeActor* CameraVolume = Cast<ACameraVolumeActor>(OtherActor);
	if (CameraVolume)
		OverlappingCameraVolumes.Remove(CameraVolume);
}

//Update with changed property
#if WITH_EDITOR
void ACameraVolumesCharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == TEXT("DefaultCameraLocation") || TEXT("DefaultCameraFocalPoint") || TEXT("DefaultCameraFieldOfView"))
	{
		UpdateCameraComponent();
	}
}
#endif

void ACameraVolumesCharacter::UpdateCameraComponent()
{
	DefaultCameraRotation = FRotationMatrix::MakeFromX(DefaultCameraFocalPoint - DefaultCameraLocation).ToQuat();
	CameraComponent->SetRelativeLocationAndRotation(DefaultCameraLocation, DefaultCameraRotation);
	CameraComponent->SetFieldOfView(DefaultCameraFieldOfView);
}