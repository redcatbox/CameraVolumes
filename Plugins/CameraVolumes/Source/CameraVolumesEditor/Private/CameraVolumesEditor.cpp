// redbox, 2021

#include "CameraVolumesEditor.h"
#include "ISettingsModule.h"
#include "CameraVolumesRuntimeSettings.h"

#define LOCTEXT_NAMESPACE "FCameraVolumesEditorModule"

DEFINE_LOG_CATEGORY(CameraVolumesEditorLog);

void FCameraVolumesEditorModule::StartupModule()
{
	RegisterSettings();
}

void FCameraVolumesEditorModule::ShutdownModule()
{
	UnregisterSettings();
}

void FCameraVolumesEditorModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "CameraVolumes",
			LOCTEXT("RuntimeSettingsName", "Camera Volumes"),
			LOCTEXT("RuntimeSettingsDescription", "Configure the Camera Volumes plugin"),
			GetMutableDefault<UCameraVolumesRuntimeSettings>());
	}
}

void FCameraVolumesEditorModule::UnregisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "CameraVolumes");
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCameraVolumesEditorModule, CameraVolumesEditor)
