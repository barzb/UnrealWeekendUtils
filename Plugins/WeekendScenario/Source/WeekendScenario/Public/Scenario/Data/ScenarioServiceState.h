///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendScenario UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "ActiveScenarioTaskData.h"
#include "SaveGame/ModularSaveGame.h"

#include "ScenarioServiceState.generated.h"

UCLASS(DisplayName = "Scenario Service State")
class WEEKENDSCENARIO_API UScenarioServiceState : public USaveGameModule
{
	GENERATED_BODY()

public:
	UScenarioServiceState()
	{
		DefaultModuleName = "ScenarioService";
		ModuleVersion = 0;
	}

	UPROPERTY(SaveGame, VisibleAnywhere, meta = (ForceInlineRow, ShowOnlyInnerProperties))
	TMap<FString, FActiveScenarioTaskData> ScenarioTaskStates = {};
};
