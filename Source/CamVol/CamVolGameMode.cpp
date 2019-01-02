// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "CamVolGameMode.h"
#include "CamVolCharacter.h"
#include "CamVolPlayerController.h"
#include "UObject/ConstructorHelpers.h"

ACamVolGameMode::ACamVolGameMode()
{
	DefaultPawnClass = ACamVolCharacter::StaticClass();
	PlayerControllerClass = ACamVolPlayerController::StaticClass();
}
