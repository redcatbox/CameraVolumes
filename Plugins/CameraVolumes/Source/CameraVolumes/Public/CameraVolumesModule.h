// redbox, 2026

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "CameraVolumesTypes.h"
#if SHOWDEBUG
#include "DisplayDebugHelpers.h"
#endif

class FCameraVolumesModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

#if SHOWDEBUG
	static void OnShowDebugInfo(class AHUD* HUD, class UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);
#endif
};
