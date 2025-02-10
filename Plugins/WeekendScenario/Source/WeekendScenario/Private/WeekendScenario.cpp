///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendScenario UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "WeekendScenario.h"

#define LOCTEXT_NAMESPACE "FWeekendScenarioModule"

DEFINE_LOG_CATEGORY(LogScenario);

void FWeekendScenarioModule::StartupModule()
{
}

void FWeekendScenarioModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FWeekendScenarioModule, WeekendScenario)
