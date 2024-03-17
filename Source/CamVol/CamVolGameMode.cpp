
#include "CamVolGameMode.h"
#include "CamVolCharacter.h"
#include "CamVolPlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CamVolGameMode)

ACamVolGameMode::ACamVolGameMode()
{
	DefaultPawnClass = ACamVolCharacter::StaticClass();
	PlayerControllerClass = ACamVolPlayerController::StaticClass();
}
