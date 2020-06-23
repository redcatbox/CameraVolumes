
#include "CamVolGameMode.h"
#include "CamVolCharacter.h"
#include "CamVolPlayerController.h"

ACamVolGameMode::ACamVolGameMode()
{
	DefaultPawnClass = ACamVolCharacter::StaticClass();
	PlayerControllerClass = ACamVolPlayerController::StaticClass();
}
