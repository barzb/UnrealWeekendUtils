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
#include "GameplayTagContainer.h"

#include "ActiveScenarioTaskData.generated.h"

USTRUCT()
struct WEEKENDSCENARIO_API FActiveScenarioTaskData
{
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame, EditDefaultsOnly)
	FGameplayTagContainer PassedCheckpoints = {};

	UPROPERTY(SaveGame, EditDefaultsOnly)
	FGameplayTag LastPassedEntryPoint = {};
};
