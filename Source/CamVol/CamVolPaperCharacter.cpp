
#include "CamVolPaperCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Camera/CameraComponent.h"

ACamVolPaperCharacter::ACamVolPaperCharacter()
{
	// Set the size of our collision capsule.
	GetCapsuleComponent()->SetCapsuleHalfHeight(96.f);
	GetCapsuleComponent()->SetCapsuleRadius(40.f);

	// Flipbooks
	static ConstructorHelpers::FObjectFinder<UPaperFlipbook> IdleAnimationObj(TEXT("/Game/2DSideScroller/Sprites/IdleAnimation.IdleAnimation"));
	if (IdleAnimationObj.Object)
		IdleAnimation = IdleAnimationObj.Object;

	static ConstructorHelpers::FObjectFinder<UPaperFlipbook> RunningAnimationObj(TEXT("/Game/2DSideScroller/Sprites/RunningAnimation.RunningAnimation"));
	if (RunningAnimationObj.Object)
		RunningAnimation = RunningAnimationObj.Object;

	GetSprite()->SetFlipbook(IdleAnimation);

	// Configure character movement
	GetCharacterMovement()->GravityScale = 2.f;
	GetCharacterMovement()->AirControl = 0.8f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->MaxFlySpeed = 600.f;
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.f, -1.f, 0.f));

	// Behave like a traditional 2D platformer character, with a flat bottom instead of a curved capsule bottom
	// Note: This can cause a little floating when going up inclines; you can choose the tradeoff between better
	// behavior on the edge of a ledge versus inclines by setting this to true or false
	GetCharacterMovement()->bUseFlatBaseForFloorChecks = true;

	// Enable replication on the Sprite component so animations show up when networked
	GetSprite()->SetIsReplicated(true);
	bReplicates = true;

	// Orthographic camera
	GetCameraComponent()->ProjectionMode = ECameraProjectionMode::Orthographic;
	GetCameraComponent()->OrthoWidth = 2048.f;

	// Camera lag
	GetCameraComponent()->bEnableCameraLocationLag = true;
	GetCameraComponent()->CameraLocationLagSpeed = 5.f;
	GetCameraComponent()->bEnableCameraRotationLag = true;
	GetCameraComponent()->CameraRotationLagSpeed = 5.f;
	GetCameraComponent()->bEnableCameraOrthoWidthInterp = true;
	GetCameraComponent()->CameraOrthoWidthInterpSpeed = 5.f;
}

// Animation
void ACamVolPaperCharacter::UpdateAnimation()
{
	const FVector PlayerVelocity = GetVelocity();
	const float PlayerSpeedSqr = PlayerVelocity.SizeSquared();

	// Are we moving or standing still?
	UPaperFlipbook* DesiredAnimation = (PlayerSpeedSqr > 0.0f) ? RunningAnimation : IdleAnimation;
	if(GetSprite()->GetFlipbook() != DesiredAnimation)
		GetSprite()->SetFlipbook(DesiredAnimation);
}

void ACamVolPaperCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateCharacter();	
}

// Input
void ACamVolPaperCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACamVolPaperCharacter::MoveRight);
}

void ACamVolPaperCharacter::MoveRight(float Value)
{
	// Apply the input to the character motion
	AddMovementInput(FVector(1.f, 0.f, 0.f), Value);
}

void ACamVolPaperCharacter::UpdateCharacter()
{
	// Update animation to match the motion
	UpdateAnimation();

	// Now setup the rotation of the controller based on the direction we are travelling
	const FVector PlayerVelocity = GetVelocity();	
	float TravelDirection = PlayerVelocity.X;
	// Set the rotation so that the character faces his direction of travel.
	if (Controller)
	{
		if (TravelDirection > 0.f)
			Controller->SetControlRotation(FRotator(0.f, 0.f, 0.f));
		else if (TravelDirection < 0.f)
			Controller->SetControlRotation(FRotator(0.f, 180.f, 0.f));
	}
}
