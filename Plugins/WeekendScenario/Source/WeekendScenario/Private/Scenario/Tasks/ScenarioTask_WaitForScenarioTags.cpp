///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendScenario UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Scenario/Tasks/ScenarioTask_WaitForScenarioTags.h"

#include "Scenario/ScenarioService.h"

UScenarioTask_WaitForScenarioTags* UScenarioTask_WaitForScenarioTags::WaitForScenarioTag(TScriptInterface<IGameplayTaskOwnerInterface> TaskOwner, FGameplayTag TagToWaitFor, FName TaskName)
{
	UScenarioTask_WaitForScenarioTags* NewTask = NewScenarioTask<UScenarioTask_WaitForScenarioTags>(TaskOwner, TaskName);
	NewTask->TagsQuery = FGameplayTagQuery::MakeQuery_MatchAllTags(TagToWaitFor.GetSingleTagContainer());
	return NewTask;
}

UScenarioTask_WaitForScenarioTags* UScenarioTask_WaitForScenarioTags::WaitForScenarioTags(TScriptInterface<IGameplayTaskOwnerInterface> TaskOwner, FGameplayTagQuery TagsQuery, FName TaskName)
{
	UScenarioTask_WaitForScenarioTags* NewTask = NewScenarioTask<UScenarioTask_WaitForScenarioTags>(TaskOwner, TaskName);
	NewTask->TagsQuery = TagsQuery;
	return NewTask;
}

void UScenarioTask_WaitForScenarioTags::Activate()
{
	Super::Activate();

	CheckScenarioTagsRequirement();

	if (IsRunning())
	{
		UseScenarioService().OnScenarioTagsChanged().AddUObject(this, &ThisClass::CheckScenarioTagsRequirement);
	}
}

void UScenarioTask_WaitForScenarioTags::Cleanup()
{
	UseScenarioService().OnScenarioTagsChanged().RemoveAll(this);

	Super::Cleanup();
}

FString UScenarioTask_WaitForScenarioTags::GetDebugString() const
{
	switch (TaskState)
	{
		case EGameplayTaskState::Active:
			return FString::Printf(TEXT("Waiting.. (%.1fs)"), GetRuntime());
		case EGameplayTaskState::Paused:
			return FString::Printf(TEXT("Paused (%.1fs)"), GetRuntime());
		case EGameplayTaskState::Finished:
			return FString::Printf(TEXT("Completed (%.1fs)"), GetRuntime());
		default: return "";
	}
}

void UScenarioTask_WaitForScenarioTags::CheckScenarioTagsRequirement()
{
	FGameplayTagContainer ScenarioTags;
	UseGameService<UScenarioService>(this)
	.GetScenarioTagsProvider()->GetOwnedGameplayTags(OUT ScenarioTags);

	if (TagsQuery.Matches(ScenarioTags))
	{
		Complete();
	}
}
