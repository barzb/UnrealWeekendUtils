///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "GameplayDebugger/GameplayDebuggerUtils.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGameService, Log, All);
DECLARE_STATS_GROUP(TEXT("Game Service"), STATGROUP_GameService, STATCAT_Advanced);

class FWeekendUtilsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
