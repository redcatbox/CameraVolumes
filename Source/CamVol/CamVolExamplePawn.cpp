// Dmitriy Barannik aka redbox, 2019

#include "CamVolExamplePawn.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "CameraVolumesCameraManager.h"

ACamVolExamplePawn::ACamVolExamplePawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create collision capsule
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->InitCapsuleSize(50.0f, 50.0f);
	CapsuleComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	CapsuleComponent->CanCharacterStepUpOn = ECB_No;
	CapsuleComponent->SetShouldUpdatePhysicsVolume(true);
	//CapsuleComponent->bCheckAsyncSceneOnMove = false;
	CapsuleComponent->SetCanEverAffectNavigation(false);
	CapsuleComponent->bDynamicObstacle = true;
	CapsuleComponent->bHiddenInGame = false;
	RootComponent = CapsuleComponent;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(CapsuleComponent);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshObj(TEXT("/Game/Geometry/Meshes/1M_Cube"));
	if (StaticMeshObj.Object)
		StaticMeshComponent->SetStaticMesh(StaticMeshObj.Object);

	// Bind overlap events
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ACamVolExamplePawn::OnCapsuleComponentBeginOverlapDelegate);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &ACamVolExamplePawn::OnCapsuleComponentEndOverlapDelegate);

	// Create a camera
	CameraComponent = CreateDefaultSubobject<UCameraVolumesCameraComponent>(TEXT("Camera"));
	CameraComponent->bUsePawnControlRotation = false; // We don't want the controller rotating the camera
	CameraComponent->bEnableCameraLocationLag = true;
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
	// set up gameplay key bindings
	PlayerInputComponent->BindAxis("MoveRight", this, &ACamVolExamplePawn::MoveRight);
	PlayerInputComponent->BindAxis("MoveForward", this, &ACamVolExamplePawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveUp", this, &ACamVolExamplePawn::MoveUp);
}

void ACamVolExamplePawn::MoveRight(float Value)
{
	// add offset in that direction
	AddActorLocalOffset(FVector(1.f, 0.f, 0.f) * Value * 5.f, true);
}

void ACamVolExamplePawn::MoveForward(float Value)
{
	// add offset in that direction
	AddActorLocalOffset(FVector(0.f, 1.f, 0.f) * Value * 5.f, true);
}

void ACamVolExamplePawn::MoveUp(float Value)
{
	// add offset in that direction
	AddActorLocalOffset(FVector(0.f, 0.f, 1.f) * Value * 5.f, true);
}

void ACamVolExamplePawn::OnCapsuleComponentBeginOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACameraVolumeActor* CameraVolume = Cast<ACameraVolumeActor>(OtherActor);
	if (CameraVolume)
	{
		GetCameraComponent()->OverlappingCameraVolumes.AddUnique(CameraVolume);

		// Update camera volumes check condition in PlayerCameraManager
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			ACameraVolumesCameraManager* CameraVolumePCM = Cast<ACameraVolumesCameraManager>(PC->PlayerCameraManager);
			if (CameraVolumePCM)
				CameraVolumePCM->SetCheckCameraVolumes(true);
		}
	}
}

void ACamVolExamplePawn::OnCapsuleComponentEndOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACameraVolumeActor* CameraVolume = Cast<ACameraVolumeActor>(OtherActor);
	if (CameraVolume)
		GetCameraComponent()->OverlappingCameraVolumes.Remove(CameraVolume);
}

UCameraVolumesCameraComponent* ACamVolExamplePawn::GetCameraComponent() const
{
	return CameraComponent;
}