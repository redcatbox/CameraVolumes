
#include "TwoPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "CameraVolumesCameraManager.h"

ATwoPlayerCharacter::ATwoPlayerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(25.f, 25.f);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	//GetCapsuleComponent()->bHiddenInGame = false;

	// Camera lag
	GetCameraComponent()->bEnableCameraLocationLag = true;
	GetCameraComponent()->CameraLocationLagSpeed = 5.f;
	GetCameraComponent()->bEnableCameraRotationLag = true;
	GetCameraComponent()->CameraRotationLagSpeed = 5.f;
	GetCameraComponent()->bEnableCameraFOVInterp = true;
	GetCameraComponent()->CameraFOVInterpSpeed = 5.f;

	CharacterA = nullptr;
	CharacterB = nullptr;

	RightDirection = FVector::ZeroVector;
	ForwardDirection = FVector::ZeroVector;
}

void ATwoPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	CharacterA = GetWorld()->SpawnActor<ATwoCharacter>(GetActorLocation() + FVector(-100.f, 0.f, 0.f), FRotator::ZeroRotator);
	CharacterA->SpawnDefaultController();
	CharacterB = GetWorld()->SpawnActor<ATwoCharacter>(GetActorLocation() + FVector(100.f, 0.f, 0.f), FRotator::ZeroRotator);
	CharacterB->SpawnDefaultController();
}

void ATwoPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (CharacterA && CharacterB)
		SetActorLocation((CharacterA->GetActorLocation() + CharacterB->GetActorLocation()) * 0.5f);

	RightDirection = GetCameraComponent()->GetRightVector();
	RightDirection.Z = 0;
	RightDirection.Normalize();
	ForwardDirection = GetCameraComponent()->GetForwardVector();
	ForwardDirection.Z = 0;
	ForwardDirection.Normalize();
}

void ATwoPlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAxis("MoveRightA", this, &ATwoPlayerCharacter::MoveRightA);
	PlayerInputComponent->BindAxis("MoveForwardA", this, &ATwoPlayerCharacter::MoveForwardA);
	PlayerInputComponent->BindAxis("MoveRightB", this, &ATwoPlayerCharacter::MoveRightB);
	PlayerInputComponent->BindAxis("MoveForwardB", this, &ATwoPlayerCharacter::MoveForwardB);
}

void ATwoPlayerCharacter::MoveRightA(float Value)
{
	if (CharacterA)
		CharacterA->AddMovementInput(RightDirection, Value);
}

void ATwoPlayerCharacter::MoveForwardA(float Value)
{
	if (CharacterA)
		CharacterA->AddMovementInput(ForwardDirection, Value);
}

void ATwoPlayerCharacter::MoveRightB(float Value)
{
	if (CharacterB)
		CharacterB->AddMovementInput(RightDirection, Value);
}

void ATwoPlayerCharacter::MoveForwardB(float Value)
{
	if (CharacterB)
		CharacterB->AddMovementInput(ForwardDirection, Value);
}