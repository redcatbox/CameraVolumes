// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
// Dmitriy Barannik aka redbox, 2019

#include "CamVolCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CamVolPlayerCameraManager.h"

ACamVolCharacter::ACamVolCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ACamVolCharacter::OnCapsuleComponentBeginOverlapDelegate);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &ACamVolCharacter::OnCapsuleComponentEndOverlapDelegate);

	// Mesh
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -96.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshObj(TEXT("/Game/Mannequin/Character/Mesh/SK_Mannequin"));
	if (SkeletalMeshObj.Object)
	{
		GetMesh()->SetSkeletalMesh(SkeletalMeshObj.Object, true);
	}
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimClass(TEXT("/Game/Mannequin/Animations/ThirdPerson_AnimBP"));
	if (AnimClass.Class)
	{
		GetMesh()->AnimClass = AnimClass.Class;
	}

	// Don't rotate when the controller rotates.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Side-scroller camera defaults
	DefaultCameraLocation = FVector(1000.f, 0.f, 0.f);
	DefaultCameraRotation = FRotator(0.f, 180.f, 0.f);
	DefaultCameraFieldOfView = 90.f;

	// Top-down camera defaults
	//DefaultCameraLocation = FVector(0.f, 0.f, 1000.f);
	//DefaultCameraRotation = FRotator(-90.f, 180.f, 0.f);
	//DefaultCameraFieldOfView = 90.f;

	// Camera lag
	bEnableCameraLocationLag = false;
	CameraLocationLagSpeed = 10.0f;
	bEnableCameraRotationLag = false;
	CameraRotationLagSpeed = 10.0f;

	// Create a camera
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->bUsePawnControlRotation = false; // We don't want the controller rotating the camera
	CameraComponent->SetRelativeLocationAndRotation(DefaultCameraLocation, DefaultCameraRotation.Quaternion());
	CameraComponent->SetFieldOfView(DefaultCameraFieldOfView);

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Face in the direction we are moving..
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f); // ...at this rotation rate
	GetCharacterMovement()->GravityScale = 2.f;
	GetCharacterMovement()->AirControl = 0.8f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->MaxFlySpeed = 600.f;
}

// Input
void ACamVolCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	PlayerInputComponent->BindAxis("MoveRight", this, &ACamVolCharacter::MoveRight);
	PlayerInputComponent->BindAxis("MoveForward", this, &ACamVolCharacter::MoveForward);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ACamVolCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ACamVolCharacter::TouchStopped);
}

void ACamVolCharacter::MoveRight(float Value)
{
	// add movement in that direction
	AddMovementInput(FVector(0.f, -1.f, 0.f), Value);
}

void ACamVolCharacter::MoveForward(float Value)
{
	// add movement in that direction
	AddMovementInput(FVector(1.f, 0.f, 0.f), Value);
}

void ACamVolCharacter::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// jump on any touch
	Jump();
}

void ACamVolCharacter::TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	StopJumping();
}

void ACamVolCharacter::UpdateCamera(FVector& CameraLocation, FRotator& CameraRotation, float CameraFOV)
{
	if (CameraComponent)
	{
		CameraComponent->SetWorldLocation(CameraLocation);
		CameraComponent->SetWorldRotation(CameraRotation);
		CameraComponent->SetFieldOfView(CameraFOV);
	}
}

void ACamVolCharacter::OnCapsuleComponentBeginOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACameraVolumeActor* CamVol = Cast<ACameraVolumeActor>(OtherActor);
	if (CamVol)
	{
		OverlappingCameraVolumes.AddUnique(CamVol);

		// Update camera volumes check condition in PlayerCameraManager
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			ACamVolPlayerCameraManager* CamVolPCM = Cast<ACamVolPlayerCameraManager>(PC->PlayerCameraManager);
			if (CamVolPCM)
				CamVolPCM->bCheckCameraVolumes = true;
		}
	}
}

void ACamVolCharacter::OnCapsuleComponentEndOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACameraVolumeActor* CamVol = Cast<ACameraVolumeActor>(OtherActor);
	if (CamVol)
	{
		OverlappingCameraVolumes.Remove(CamVol);
	}
}