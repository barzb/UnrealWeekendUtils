///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "WeekendGameService.h"

#if WITH_GAMEPLAY_DEBUGGER
 #include "GameplayDebugger/Categories/GameplayDebuggerCategory_GameServices.h"
#endif

#define LOCTEXT_NAMESPACE "FWeekendGameServiceModule"

DEFINE_LOG_CATEGORY(LogGameService);

void FWeekendGameServiceModule::StartupModule()
{
#if WITH_GAMEPLAY_DEBUGGER
	if (IGameplayDebugger::IsAvailable())
	{
		// Categories:
		FGameplayDebugger::RegisterCategory<FGameplayDebuggerCategory_GameServices>();
	}
#endif
}

void FWeekendGameServiceModule::ShutdownModule()
{
#if WITH_GAMEPLAY_DEBUGGER
	if (IGameplayDebugger::IsAvailable())
	{
		// Categories:
		FGameplayDebugger::UnregisterCategory<FGameplayDebuggerCategory_GameServices>();
	}
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FWeekendGameServiceModule, WeekendGameService)