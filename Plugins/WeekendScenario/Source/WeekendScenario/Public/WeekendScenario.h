///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2024 by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendScenario UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

WEEKENDSCENARIO_API DECLARE_LOG_CATEGORY_EXTERN(LogScenario, Log, All);

class FWeekendScenarioModule : public IModuleInterface
{
public:
	// - IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// --
};
