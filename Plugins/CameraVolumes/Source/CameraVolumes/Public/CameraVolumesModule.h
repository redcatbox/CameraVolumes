// redbox, 2025

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FCameraVolumesModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
