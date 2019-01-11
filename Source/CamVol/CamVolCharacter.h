// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
// Dmitriy Barannik aka redbox, 2019
/**
* Player pawn stores default camera parameters.
* Player pawn uses BeginOverlap event of it's collision primitive, to get overlapped camera volume and store it in special array of camera volume actors.
* Player pawn uses EndOverlap event of it's collision primitive, to remove camera volume from array of camera volume actors.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "CameraVolumeActor.h"
#include "CamVolCharacter.generated.h"

UCLASS(config = Game)
class ACamVolCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Side view camera */
	UPROPERTY()
		class UCameraComponent* CameraComponent;

	UFUNCTION()
		void OnCapsuleComponentBeginOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnCapsuleComponentEndOverlapDelegate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	/** Called for side to side input */
	void MoveRight(float Val);

	/** Called for front to back input */
	void MoveForward(float Val);

	/** Handle touch inputs. */
	void TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location);

	/** Handle touch stop event. */
	void TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location);

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	ACamVolCharacter();

	/** Default camera relative location */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		FVector DefaultCameraLocation;

	/** Default camera world rotation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		FRotator DefaultCameraRotation;

	/** Default camera FOV */ // Better to be set in config/game settings.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", Meta = (UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0", Units = deg))
		float DefaultCameraFieldOfView;

	/** Should camera use location lag? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Lag")
		bool bEnableCameraLocationLag;

	/** Camera location lag speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Lag", Meta = (EditCondition = "bEnableCameraLocationLag", ClampMin = "0.1", ClampMax = "100.0", UIMin = "0.1", UIMax = "100.0"))
		float CameraLocationLagSpeed;

	/** Should camera use rotation lag? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Lag")
		bool bEnableCameraRotationLag;

	/** Camera rotation lag speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Lag", Meta = (EditCondition = "bEnableCameraRotationLag", ClampMin = "0.1", ClampMax = "100.0", UIMin = "0.1", UIMax = "100.0"))
		float CameraRotationLagSpeed;

	UFUNCTION()
		virtual void UpdateCamera(FVector& CameraLocation, FRotator& CameraRotation, float CameraFOV);

	UPROPERTY()
		TArray<ACameraVolumeActor*> OverlappingCameraVolumes;

	/** Returns SideViewCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetCameraComponent() const { return CameraComponent; }
};
