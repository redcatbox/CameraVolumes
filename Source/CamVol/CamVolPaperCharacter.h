
#pragma once

#include "CameraVolumesPaperCharacter.h"
#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"
#include "CamVolPaperCharacter.generated.h"

UCLASS(Config = Game)
class ACamVolPaperCharacter : public ACameraVolumesPaperCharacter
{
	GENERATED_BODY()

public:
	ACamVolPaperCharacter();
	virtual void Tick(float DeltaSeconds) override;

protected:
	// The animation to play while running around
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	class UPaperFlipbook* RunningAnimation;

	// The animation to play while idle (standing still)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	class UPaperFlipbook* IdleAnimation;

	// Called to choose the correct animation to play based on the character's movement state
	void UpdateAnimation();

	// Called for side to side input
	void MoveRight(float Value);

	// Handle touch inputs.
	void TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location);

	// Handle touch stop event.
	void TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location);

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

	void UpdateCharacter();
};
