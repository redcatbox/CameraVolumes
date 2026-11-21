
#include "CamVolExamplePawn.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "CameraVolumesCameraManager.h"
#include "CameraVolumeActor.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CamVolExamplePawn)

ACamVolExamplePawn::ACamVolExamplePawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	// Create collision capsule
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->InitCapsuleSize(50.0f, 50.0f);
	CapsuleComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	CapsuleComponent->CanCharacterStepUpOn = ECB_No;
	CapsuleComponent->SetShouldUpdatePhysicsVolume(true);
	CapsuleComponent->SetCanEverAffectNavigation(false);
	CapsuleComponent->bDynamicObstacle = true;
	CapsuleComponent->bHiddenInGame = false;
	RootComponent = CapsuleComponent;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(CapsuleComponent);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshObj(TEXT("/Game/Geometry/Meshes/1M_Cube"));
	if (StaticMeshObj.Object)
	{
		StaticMeshComponent->SetStaticMesh(StaticMeshObj.Object);
	}

	// Create a camera
	CameraComponent = CreateDefaultSubobject<UCameraVolumesCameraComponent>(TEXT("Camera"));
	CameraComponent->bUsePawnControlRotation = false; // We don't want the controller rotating the camera
	CameraComponent->bEnableCameraLocationLag = true;
}

void ACamVolExamplePawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Bind overlap events
	GetCollisionPrimitiveComponent()->OnComponentBeginOverlap.AddDynamic(this, &ACamVolExamplePawn::OnCollisionPrimitiveComponentBeginOverlap);
	GetCollisionPrimitiveComponent()->OnComponentEndOverlap.AddDynamic(this, &ACamVolExamplePawn::OnCollisionPrimitiveComponentEndOverlap);
}

void ACamVolExamplePawn::BeginPlay()
{
	Super::BeginPlay();
}

void ACamVolExamplePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACamVolExamplePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
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
		Input->BindAction(MoveHorizontalAction, ETriggerEvent::Triggered, this, &ThisClass::MoveHorizontal);
		Input->BindAction(MoveVerticalAction, ETriggerEvent::Triggered, this, &ThisClass::MoveVertical);
	}
}

void ACamVolExamplePawn::MoveHorizontal(const FInputActionValue& Value)
{
	FVector2D InputValue = Value.Get<FVector2D>();
	AddActorLocalOffset(FVector(1.f, 0.f, 0.f) * InputValue.X * 5.f, true);
	AddActorLocalOffset(FVector(0.f, 1.f, 0.f) * InputValue.Y * 5.f, true);
}

void ACamVolExamplePawn::MoveVertical(const FInputActionValue& Value)
{
	float InputValue = Value.Get<float>();
	AddActorLocalOffset(FVector(0.f, 0.f, 1.f) * InputValue * 5.f, true);
}

void ACamVolExamplePawn::OnCollisionPrimitiveComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ACameraVolumeActor* CameraVolume = Cast<ACameraVolumeActor>(OtherActor))
	{
		if (GetCameraComponent()->OverlappingCameraVolumes.Num() == 0)
		{
			// Update camera volumes check condition in PlayerCameraManager
			if (APlayerController* PC = Cast<APlayerController>(GetController()))
			{
				if (ACameraVolumesCameraManager* CameraVolumePCM = Cast<ACameraVolumesCameraManager>(PC->PlayerCameraManager))
				{
					CameraVolumePCM->SetProcessCameraVolumes(true);
				}
			}
		}

		GetCameraComponent()->OverlappingCameraVolumes.Emplace(CameraVolume);
	}
}

void ACamVolExamplePawn::OnCollisionPrimitiveComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ACameraVolumeActor* CameraVolume = Cast<ACameraVolumeActor>(OtherActor))
	{
		GetCameraComponent()->OverlappingCameraVolumes.Remove(CameraVolume);
	}
}

UCameraVolumesCameraComponent* ACamVolExamplePawn::GetCameraComponent() const
{
	return CameraComponent;
}

UPrimitiveComponent* ACamVolExamplePawn::GetCollisionPrimitiveComponent() const
{
	return CapsuleComponent;
}
