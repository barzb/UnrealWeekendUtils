///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendCustomization UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "WeekendCustomization.h"

#include "Cyborg/CyborgSlotGroupComponent.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebugger/GameplayDebuggerUtils.h"
#include "Cyborg/Debug/GameplayDebuggerCategory_CyborgSlots.h"
#endif

#define LOCTEXT_NAMESPACE "FWeekendCustomizationModule"

DEFINE_LOG_CATEGORY(LogWeekendCustomization);

// Register framework custom version for serialization compatibility:
const FGuid FCyborgFrameworkVersion::GUID(0x4EECE3DC, 0x4EECE3DC, 0x4EECE3DC, 0x4EECE3DC);
FDevVersionRegistration GRegisterCyborgFrameworkObjectVersion(FCyborgFrameworkVersion::GUID, FCyborgFrameworkVersion::LatestVersion, TEXT("Weekend-CyborgFramework"));

void FWeekendCustomizationModule::StartupModule()
{
#if WITH_GAMEPLAY_DEBUGGER
	if (IGameplayDebugger::IsAvailable())
	{
		FGameplayDebugger::RegisterCategory<FGameplayDebuggerCategory_CyborgSlots>();
	}
#endif

	RegisterComponentVisualizer(UCyborgSlotGroupComponent::StaticClass()->GetFName(), MakeShareable(new UCyborgSlotGroupComponent::FComponentVisualizer));
}

void FWeekendCustomizationModule::ShutdownModule()
{
#if WITH_GAMEPLAY_DEBUGGER
	if (IGameplayDebugger::IsAvailable())
	{
		FGameplayDebugger::UnregisterCategory<FGameplayDebuggerCategory_CyborgSlots>();
	}
#endif

	UnregisterComponentVisualizer(UCyborgSlotGroupComponent::StaticClass()->GetFName());
}

void FWeekendCustomizationModule::RegisterComponentVisualizer(const FName ComponentClassName, const TSharedPtr<FComponentVisualizer> Visualizer)
{
	if (GUnrealEd != nullptr && Visualizer.IsValid())
	{
		GUnrealEd->RegisterComponentVisualizer(ComponentClassName, Visualizer);
		Visualizer->OnRegister();
	}
}

void FWeekendCustomizationModule::UnregisterComponentVisualizer(const FName ComponentClassName)
{
	if (GUnrealEd != nullptr)
	{
		GUnrealEd->UnregisterComponentVisualizer(ComponentClassName);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FWeekendCustomizationModule, WeekendCustomization)
