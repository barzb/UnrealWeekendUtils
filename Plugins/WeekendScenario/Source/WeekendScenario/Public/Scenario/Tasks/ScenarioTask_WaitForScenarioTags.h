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
#include "AsyncScenarioTask.h"
#include "UObject/Object.h"

#include "ScenarioTask_WaitForScenarioTags.generated.h"

/**
 * #todo-docs
 */
UCLASS(meta = (ExposedAsyncProxy = "WaitTask"))
class WEEKENDSCENARIO_API UScenarioTask_WaitForScenarioTags : public UAsyncScenarioTask
{
	GENERATED_UCLASS_BODY()

public:
	///////////////////////////////////////////////////////////////////////////////////////
	/// TASK FACTORIES

	UFUNCTION(BlueprintCallable, Category = "Scenario", meta = (DefaultToSelf = "TaskOwner", HidePin = "TaskOwner", HideResult, BlueprintInternalUseOnly = "TRUE"))
	static UScenarioTask_WaitForScenarioTags* WaitForScenarioTag(TScriptInterface<IGameplayTaskOwnerInterface> TaskOwner, FGameplayTag TagToWaitFor, FName TaskName = "WaitForTag");

	UFUNCTION(BlueprintCallable, Category = "Scenario", meta = (DefaultToSelf = "TaskOwner", HidePin = "TaskOwner", HideResult, BlueprintInternalUseOnly = "TRUE", Keywords = "Query"))
	static UScenarioTask_WaitForScenarioTags* WaitForScenarioTags(TScriptInterface<IGameplayTaskOwnerInterface> TaskOwner, FGameplayTagQuery TagsQuery, FName TaskName = "WaitForTags");

	///////////////////////////////////////////////////////////////////////////////////////

protected:
	UPROPERTY()
	FGameplayTagQuery TagsQuery = {};

	///////////////////////////////////////////////////////////////////////////////////////
	/// OVERRIDES

	// - UAsyncScenarioTask
	virtual void Activate() override;
	virtual void Cleanup() override;
	virtual FString GetDebugString() const override;
	// --

	///////////////////////////////////////////////////////////////////////////////////////

	virtual void CheckScenarioTagsRequirement();
};
