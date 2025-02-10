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
#include "Engine/DeveloperSettings.h"
#include "Scenario/Components/ScenarioTasksComponent.h"

#include "ScenarioProjectSettings.generated.h"

/**
 * #todo-docs
 */
UCLASS(Config = "Scenario", DefaultConfig, meta = (DisplayName = "Weekend Scenarios"))
class WEEKENDSCENARIO_API UScenarioProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Scenario Service", NoClear)
	TSubclassOf<UScenarioTasksComponent> ScenarioTasksComponentClass = UScenarioTasksComponent::StaticClass();
};
