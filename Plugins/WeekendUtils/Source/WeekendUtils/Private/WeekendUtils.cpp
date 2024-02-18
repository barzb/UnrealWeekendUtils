///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "WeekendUtils.h"

#if WITH_GAMEPLAY_DEBUGGER
 #include "GameplayDebugger/Categories/GameplayDebuggerCategory_GameFeatures.h"
 #include "GameplayDebugger/Categories/GameplayDebuggerCategory_GameServices.h"
 #include "GameplayDebugger/Categories/GameplayDebuggerCategory_InputActionAbilities.h"
 #include "GameplayDebugger/Extensions/GameplayDebuggerExtension_SelfDebugActor.h"
 #include "GameplayDebugger/Extensions/GameplayDebuggerExtension_ToggleUiVisibility.h"
 #include "GameplayDebugger/GameplayDebuggerUtils.h"
#endif

#define LOCTEXT_NAMESPACE "FWeekendUtilsModule"

DEFINE_LOG_CATEGORY(LogGameService);

void FWeekendUtilsModule::StartupModule()
{
#if WITH_GAMEPLAY_DEBUGGER
	if (IGameplayDebugger::IsAvailable())
	{
		// Categories:
		FGameplayDebugger::RegisterCategory<FGameplayDebuggerCategory_GameServices>();
		FGameplayDebugger::RegisterCategory<FGameplayDebuggerCategory_GameFeatures>();
		FGameplayDebugger::RegisterCategory<FGameplayDebuggerCategory_InputActionAbilities>();

		// Extensions:
		FGameplayDebugger::RegisterExtension<FGameplayDebuggerExtension_SelfDebugActor>();
		FGameplayDebugger::RegisterExtension<FGameplayDebuggerExtension_ToggleUiVisibility>();
	}
#endif
}

void FWeekendUtilsModule::ShutdownModule()
{
#if WITH_GAMEPLAY_DEBUGGER
	if (IGameplayDebugger::IsAvailable())
	{
		// Categories:
		FGameplayDebugger::UnregisterCategory<FGameplayDebuggerCategory_GameServices>();
		FGameplayDebugger::UnregisterCategory<FGameplayDebuggerCategory_GameFeatures>();
		FGameplayDebugger::UnregisterCategory<FGameplayDebuggerCategory_InputActionAbilities>();

		// Extensions:
		FGameplayDebugger::UnregisterExtension<FGameplayDebuggerExtension_SelfDebugActor>();
		FGameplayDebugger::UnregisterExtension<FGameplayDebuggerExtension_ToggleUiVisibility>();
	}
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FWeekendUtilsModule, WeekendUtils)
