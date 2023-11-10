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
 #include "GameService/Debug/GameplayDebuggerCategory_GameServices.h"
 #include "GameplayDebugger.h"
 #include "GameplayDebugger/Categories/GameplayDebuggerCategory_GameFeatures.h"
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
		// Categories:
		FGameplayDebugger::RegisterCategory<FGameplayDebuggerCategory_GameServices>();
		FGameplayDebugger::RegisterCategory<FGameplayDebuggerCategory_GameFeatures>();

		// Extensions:
		FGameplayDebugger::RegisterExtension<FGameplayDebuggerExtension_SelfDebugActor>();
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
		// Categories:
		FGameplayDebugger::UnregisterCategory<FGameplayDebuggerCategory_GameServices>();
		FGameplayDebugger::UnregisterCategory<FGameplayDebuggerCategory_GameFeatures>();

		// Extensions:
		FGameplayDebugger::UnregisterExtension<FGameplayDebuggerExtension_SelfDebugActor>();
	}
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FWeekendUtilsModule, WeekendUtils)
