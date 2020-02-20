//Dmitriy Barannik aka redbox, 2019

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FCameraVolumesModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};