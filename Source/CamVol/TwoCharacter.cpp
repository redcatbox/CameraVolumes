
#include "TwoCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CameraVolumesCameraManager.h"

ATwoCharacter::ATwoCharacter()
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
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}