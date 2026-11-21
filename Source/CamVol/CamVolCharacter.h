
#pragma once

#include "CameraVolumesCharacter.h"
#include "CamVolCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS(Config = Game)
class ACamVolCharacter : public ACameraVolumesCharacter
{
	GENERATED_BODY()

public:
	ACamVolCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnhancedInput")
	UInputMappingContext* InputMapping;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnhancedInput")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnhancedInput")
	UInputAction* JumpAction;

protected:
	UFUNCTION()
	void MoveInput(const FInputActionValue& Value);

	UFUNCTION()
	void JumpInput(const FInputActionValue& Value);

	void TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location);
};
