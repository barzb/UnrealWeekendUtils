///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendScenario UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Scenario/Tasks/ScenarioTask_CreateCheckpoint.h"

#include "SaveGame/SaveGameService.h"
#include "Scenario/ScenarioService.h"
#include "WeekendScenario.h"

UScenarioTask_CreateCheckpoint* UScenarioTask_CreateCheckpoint::CreateCheckpoint(UAsyncScenarioTask* TaskOwner, FGameplayTag EntryPoint, bool bSaveGame, FName TaskName)
{
	UScenarioTask_CreateCheckpoint* NewTask = NewScenarioTask<UScenarioTask_CreateCheckpoint>(*TaskOwner, TaskName);
	NewTask->EntryPointTag = EntryPoint;
	NewTask->bShouldSaveGame = bSaveGame;
	return NewTask;
}

void UScenarioTask_CreateCheckpoint::Activate()
{
	Super::Activate();

	GetTypedOuter<UAsyncScenarioTask>()->CreateCheckpoint(EntryPointTag);

	if (!bShouldSaveGame)
	{
		Complete();
		return;
	}

	SaveGameService = FindOptionalGameService<USaveGameService>();
	if (!SaveGameService.IsValid())
	{
		UE_LOG(LogScenario, Log, TEXT("Checkpoint could not save the game: No SaveGameService found (%s)"), *GetPathName());
		Complete();
		return;
	}

	SaveGameService->RequestAutosave("UScenarioTask_CreateCheckpoint::Activate",
		FOnSaveLoadCompleted::CreateWeakLambda(this, [this](USaveGame*, bool)
		{
			Complete();
		}));
}

void UScenarioTask_CreateCheckpoint::Cleanup()
{
	SaveGameService.Reset();

	Super::Cleanup();
}

FString UScenarioTask_CreateCheckpoint::GetDebugString() const
{
	switch (TaskState)
	{
		case EGameplayTaskState::Active:
			return FString::Printf(TEXT("In Progress.. (%.1fs)"), GetRuntime());
		case EGameplayTaskState::Finished:
			return FString::Printf(TEXT("Created"));
		default:
			return "???";
	}
}
