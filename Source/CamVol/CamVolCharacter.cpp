
#include "CamVolCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CameraVolumesCameraManager.h"

ACamVolCharacter::ACamVolCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	// Mesh
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -96.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshObj(TEXT("/Game/Mannequin/Character/Mesh/SK_Mannequin"));
	if (SkeletalMeshObj.Object)
		GetMesh()->SetSkeletalMesh(SkeletalMeshObj.Object, true);

	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimClass(TEXT("/Game/Mannequin/Animations/ThirdPerson_AnimBP"));
	if (AnimClass.Class)
		GetMesh()->AnimClass = AnimClass.Class;

	// Configure character movement
	GetCharacterMovement()->GravityScale = 2.f;
	GetCharacterMovement()->AirControl = 0.8f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->MaxFlySpeed = 600.f;

	// Camera lag
	GetCameraComponent()->bEnableCameraLocationLag = true;
	GetCameraComponent()->CameraLocationLagSpeed = 5.f;
	GetCameraComponent()->bEnableCameraRotationLag = true;
	GetCameraComponent()->CameraRotationLagSpeed = 5.f;
	GetCameraComponent()->bEnableCameraFOVInterp = true;
	GetCameraComponent()->CameraFOVInterpSpeed = 5.f;
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
	AddMovementInput(FVector(1.f, 0.f, 0.f), Value);
}

void ACamVolCharacter::MoveForward(float Value)
{
	// add movement in that direction
	AddMovementInput(FVector(0.f, 1.f, 0.f), Value);
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