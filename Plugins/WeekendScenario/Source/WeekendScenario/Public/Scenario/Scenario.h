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
#include "Tasks/AsyncScenarioTask.h"

#include "Scenario.generated.h"

class UGameplayTask;

/**
 * #todo-docs
 */
UCLASS(BlueprintType, Blueprintable, meta = (ExposedAsyncProxy = "Scenario"))
class WEEKENDSCENARIO_API UScenario : public UAsyncScenarioTask
{
	GENERATED_BODY()

public:
	///////////////////////////////////////////////////////////////////////////////////////
	/// TASK FACTORIES

	UFUNCTION(BlueprintCallable, Category = "Scenario", meta = (DefaultToSelf = "TaskOwner", HidePin = "TaskOwner", BlueprintInternalUseOnly = "TRUE"))
	static UScenario* RunScenario(UScenario* TaskOwner, TSubclassOf<UScenario> Class, FName TaskName = NAME_None);

	///////////////////////////////////////////////////////////////////////////////////////
	/// FLOW CONTROL

	TArray<UScenario*> GetRunningSubScenarios() const;
	int32 GetNumRunningSubScenarios() const;

protected:
	///////////////////////////////////////////////////////////////////////////////////////

	UFUNCTION(BlueprintCallable, Category = "Scenario", meta = (Latent, LatentInfo = "LatentInfo"))
	void WaitForRunningScenariosToComplete(FLatentActionInfo LatentInfo);

	//#todo-scenario AddScenarioTag(..) -> TasksComponent
	//#todo-scenario AddWorldStateTag(..) -> ??? (BPFL?)
};
