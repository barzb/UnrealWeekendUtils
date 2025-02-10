///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendScenario UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "WeekendScenarioEditor.h"

#include "BlueprintNodes/AsyncScenarioTaskGraphNodeFactory.h"

#define LOCTEXT_NAMESPACE "FWeekendScenarioEditorModule"

namespace
{
	TSharedPtr<FAsyncScenarioTaskGraphNodeFactory> GAsyncScenarioTaskGraphNodeFactory = nullptr;
}

void FWeekendScenarioEditorModule::StartupModule()
{
	GAsyncScenarioTaskGraphNodeFactory = MakeShared<FAsyncScenarioTaskGraphNodeFactory>();
	FEdGraphUtilities::RegisterVisualNodeFactory(GAsyncScenarioTaskGraphNodeFactory);
}

void FWeekendScenarioEditorModule::ShutdownModule()
{
	if (GAsyncScenarioTaskGraphNodeFactory.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(GAsyncScenarioTaskGraphNodeFactory);
		GAsyncScenarioTaskGraphNodeFactory.Reset();
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FWeekendScenarioEditorModule, WeekendScenarioEditor)
