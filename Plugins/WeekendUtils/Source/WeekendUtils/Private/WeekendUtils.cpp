// (c) by Benjamin Barz

#include "WeekendUtils.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebugger.h"
#include "GameplayDebugger/Extensions/GameplayDebuggerExtension_SelfDebugActor.h"
#include "GameplayDebugger/GameplayDebuggerUtils.h"
#include "ServiceLocator/Debug/GameplayDebuggerCategory_ServiceLocator.h"
#endif

#define LOCTEXT_NAMESPACE "FWeekendUtilsModule"

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

		// Categories:
		FGameplayDebugger::RegisterCategory<FGameplayDebuggerCategory_ServiceLocator>();
		GameplayDebugger.NotifyCategoriesChanged();
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

		// Categories:
		FGameplayDebugger::UnregisterCategory<FGameplayDebuggerCategory_ServiceLocator>();
		GameplayDebugger.NotifyCategoriesChanged();
	}
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FWeekendUtilsModule, WeekendUtils)