///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz in cooperation with Nine Worlds Studios GmbH.
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See accompanying file LICENSE.
///
///////////////////////////////////////////////////////////////////////////////////////

#include "WeekendUtils.h"

#if WITH_GAMEPLAY_DEBUGGER
 #include "GameplayDebugger.h"
 #include "GameplayDebugger/Extensions/GameplayDebuggerExtension_SelfDebugActor.h"
 #include "GameplayDebugger/GameplayDebuggerUtils.h"
#endif

#define LOCTEXT_NAMESPACE "FWeekendUtilsModule"

DEFINE_LOG_CATEGORY(LogGameService);

void FWeekendUtilsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

#if WITH_GAMEPLAY_DEBUGGER
	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger& GameplayDebugger = IGameplayDebugger::Get();

		// Extensions:
		FGameplayDebugger::RegisterExtension<FGameplayDebuggerExtension_SelfDebugActor>();
		GameplayDebugger.NotifyExtensionsChanged();
	}
#endif
}

void FWeekendUtilsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

#if WITH_GAMEPLAY_DEBUGGER
	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger& GameplayDebugger = IGameplayDebugger::Get();

		// Extension:
		FGameplayDebugger::UnregisterExtension<FGameplayDebuggerExtension_SelfDebugActor>();
		GameplayDebugger.NotifyExtensionsChanged();
	}
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FWeekendUtilsModule, WeekendUtils)
