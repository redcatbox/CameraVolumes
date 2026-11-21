
#include "CamVolCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CameraVolumesCameraManager.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CamVolCharacter)

ACamVolCharacter::ACamVolCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

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

	// Configure character movement
	GetCharacterMovement()->GravityScale = 2.f;
	GetCharacterMovement()->AirControl = 0.8f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->MaxFlySpeed = 600.f;

	// Camera lag
	ACamVolCharacter::GetCameraComponent()->bEnableCameraLocationLag = true;
	ACamVolCharacter::GetCameraComponent()->CameraLocationLagSpeed = 5.f;
	ACamVolCharacter::GetCameraComponent()->bEnableCameraRotationLag = true;
	ACamVolCharacter::GetCameraComponent()->CameraRotationLagSpeed = 5.f;
	ACamVolCharacter::GetCameraComponent()->bEnableCameraFOVInterp = true;
	ACamVolCharacter::GetCameraComponent()->CameraFOVInterpSpeed = 5.f;
}

void ACamVolCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (IsValid(Subsystem))
	{
		Subsystem->ClearAllMappings();
		Subsystem->AddMappingContext(InputMapping, 0);
	}

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (IsValid(Input))
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::MoveInput);
		Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::JumpInput);
	}

	PlayerInputComponent->BindTouch(IE_Pressed, this, &ACamVolCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ACamVolCharacter::TouchStopped);
}

void ACamVolCharacter::MoveInput(const FInputActionValue& Value)
{
	FVector2D InputValue = Value.Get<FVector2D>();
	AddMovementInput(FVector(1.f, 0.f, 0.f), InputValue.X);
	AddMovementInput(FVector(0.f, 1.f, 0.f), InputValue.Y);
}

void ACamVolCharacter::JumpInput(const FInputActionValue& Value)
{
	Jump();
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
