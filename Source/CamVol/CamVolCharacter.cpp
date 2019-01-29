
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
	bEnableCameraLocationLag = true;
	CameraLocationLagSpeed = 5.f;
	bEnableCameraRotationLag = true;
	CameraRotationLagSpeed = 5.f;
	bEnableCameraFOVInterpolation = true;
	CameraFOVInterpolationSpeed = 5.f;
}

// Input
void ACamVolCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	PlayerInputComponent->BindAxis("MoveRight", this, &ACamVolCharacter::MoveRight);
	PlayerInputComponent->BindAxis("MoveForward", this, &ACamVolCharacter::MoveForward);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
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