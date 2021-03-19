
#pragma once

#include "CameraVolumesCharacter.h"
#include "CamVolCharacter.generated.h"

UCLASS(Config = Game)
class ACamVolCharacter : public ACameraVolumesCharacter
{
	GENERATED_BODY()

public:
	ACamVolCharacter();

protected:
	// Called for side to side input
	void MoveRight(float Val);

	// Called for front to back input
	void MoveForward(float Val);

	// Handle touch inputs
	void TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location);

	// Handle touch stop event
	void TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location);

	// Setup input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
};
