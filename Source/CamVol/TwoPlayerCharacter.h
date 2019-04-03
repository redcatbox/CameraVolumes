
#pragma once

#include "CameraVolumesCharacter.h"
#include "TwoCharacter.h"
#include "TwoPlayerCharacter.generated.h"

UCLASS()
class ATwoPlayerCharacter : public ACameraVolumesCharacter
{
	GENERATED_BODY()

public:
	ATwoPlayerCharacter();
	virtual void Tick(float DeltaTime) override;

 protected:
	virtual void BeginPlay() override;
 	void MoveRightA(float Val);
 	void MoveForwardA(float Val);
	void MoveRightB(float Val);
	void MoveForwardB(float Val);
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	UPROPERTY()
		ATwoCharacter* CharacterA;

	UPROPERTY()
		ATwoCharacter* CharacterB;

	UPROPERTY()
		FVector RightDirection;

	UPROPERTY()
		FVector ForwardDirection;
};
