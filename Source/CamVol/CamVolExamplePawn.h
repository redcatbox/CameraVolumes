//Dmitriy Barannik aka redbox, 2019

/**
* Example pawn support.
* Unlike Character, Pawn doesn't have collision primitive to process BeginOverlap and EndOverlap events, so it should be added.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CameraVolumesCharacterInterface.h"
#include "CamVolExamplePawn.generated.h"

UCLASS()
class CAMVOL_API ACamVolExamplePawn : public APawn, public ICameraVolumesCharacterInterface
{
	GENERATED_BODY()

	/** Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CameraVolumes, Meta = (AllowPrivateAccess = "true"))
		class UCameraVolumesCameraComponent* CameraComponent;

	virtual void OnCapsuleComponentBeginOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	virtual void OnCapsuleComponentEndOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	ACamVolExamplePawn();
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual class UCameraVolumesCameraComponent* GetCameraComponent() const override;
	virtual class UPrimitiveComponent* GetCollisionPrimitiveComponent() const override;

	/** Returns CapsuleComponent subobject **/
	FORCEINLINE class UCapsuleComponent* GetCapsuleComponent() const { return CapsuleComponent; }

protected:
	virtual void BeginPlay() override;
	void MoveRight(float Val);
	void MoveForward(float Val);
	void MoveUp(float Val);

private:
	/** The CapsuleComponent being used for movement collision (by CharacterMovement). Always treated as being vertically aligned in simple collision check functions. */
	UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UCapsuleComponent* CapsuleComponent;

	UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* StaticMeshComponent;
};
