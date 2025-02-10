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
#include "AsyncScenarioTask.h"

#include "ScenarioTask_CreateCheckpoint.generated.h"

class USaveGameService;

/**
 * #todo-docs
 */
UCLASS()
class WEEKENDSCENARIO_API UScenarioTask_CreateCheckpoint : public UAsyncScenarioTask
{
	GENERATED_BODY()

public:
	///////////////////////////////////////////////////////////////////////////////////////
	/// TASK FACTORIES

	UFUNCTION(BlueprintCallable, Category = "Scenario", meta = (DefaultToSelf = "TaskOwner", HidePin = "TaskOwner", HideResult, BlueprintInternalUseOnly = "TRUE"))
	static UScenarioTask_CreateCheckpoint* CreateCheckpoint(UAsyncScenarioTask* TaskOwner, FGameplayTag EntryPoint, bool bSaveGame = true, FName TaskName = "CreateCheckpoint");

	///////////////////////////////////////////////////////////////////////////////////////

protected:
	UPROPERTY()
	FGameplayTag EntryPointTag = {};
	bool bShouldSaveGame = false;
	TWeakObjectPtr<USaveGameService> SaveGameService = nullptr;

	///////////////////////////////////////////////////////////////////////////////////////
	/// OVERRIDES

	// - UAsyncScenarioTask
	virtual void Activate() override;
	virtual void Cleanup() override;
	virtual FString GetDebugString() const override;
	// --

	///////////////////////////////////////////////////////////////////////////////////////
};
