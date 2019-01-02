// Fill out your copyright notice in the Description page of Project Settings.

#include "CamVolPlayerController.h"
#include "CamVolPlayerCameraManager.h"

ACamVolPlayerController::ACamVolPlayerController()
{
	PlayerCameraManagerClass = ACamVolPlayerCameraManager::StaticClass();
}