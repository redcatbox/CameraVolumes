
#pragma once

#include "CameraVolumesCharacter.h"
#include "CamVolCharacter.generated.h"

UCLASS(config = Game)
class ACamVolCharacter : public ACameraVolumesCharacter
{
	GENERATED_BODY()

public:
	ACamVolCharacter();

 protected:
 	/** Called for side to side input */
 	void MoveRight(float Val);
 
 	/** Called for front to back input */
 	void MoveForward(float Val);
 
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface
};
