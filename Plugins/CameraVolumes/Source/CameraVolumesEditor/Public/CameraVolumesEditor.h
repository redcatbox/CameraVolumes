// redbox, 2021

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(CameraVolumesEditorLog, Log, All);

class FCameraVolumesEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

protected:
	void RegisterSettings();
	void UnregisterSettings();
};
