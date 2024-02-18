///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2024 by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendScenario UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Scenario/Tasks/AsyncScenarioTask.h"

#include "WeekendScenario.h"

namespace
{
	FString GetResultTagAsShortString(const FGameplayTag& GameplayTag)
	{
		const FGameplayTag& ParentTag = GameplayTag.RequestDirectParent();
		const int32 ParentTagLength = ParentTag.IsValid() ? (GetNum(ParentTag.ToString()) + 1) : 0;
		return GameplayTag.ToString().RightChop(ParentTagLength);
	}
}

TOptional<FString> UAsyncScenarioTask::GetDebugStringForChildTask(FName ChildTaskInstanceName) const
{
	for (const UGameplayTask* Task : ChildTasks)
	{
		if (Task->GetInstanceName() != ChildTaskInstanceName)
			continue;

		const FString DebugString = Task->GetDebugString();
		return DebugString.IsEmpty() ? TOptional<FString>() : DebugString;
	}
	return {};
}

TOptional<FString> UAsyncScenarioTask::GetDebugStringForChildTask(const UAsyncScenarioTask& ChildTaskRef) const
{
	for (const UGameplayTask* Task : ChildTasks)
	{
		if (Task != &ChildTaskRef)
			continue;

		const FString DebugString = Task->GetDebugString();
		return DebugString.IsEmpty() ? TOptional<FString>() : DebugString;
	}
	return {};
}

double UAsyncScenarioTask::GetRuntime() const
{
	if (!TaskStartTime.IsSet())
		return 0.0;

	if (TaskEndTime.IsSet())
		return (*TaskEndTime - *TaskStartTime);

	return (GetWorld()->GetTimeSeconds() - *TaskStartTime);
}

FString UAsyncScenarioTask::GetDebugString() const
{
	switch (TaskState)
	{
		case EGameplayTaskState::Active:
			return FString::Printf(TEXT("Running.. (%.1fs)"), GetRuntime());
		case EGameplayTaskState::Paused:
			return FString::Printf(TEXT("Paused (%.1fs)"), GetRuntime());
		case EGameplayTaskState::Finished:
			return FString::Printf(TEXT("Completed (%s)"), *GetResultTagAsShortString(CompletionResult));
		default: return "";
	}
}

UGameplayTasksComponent* UAsyncScenarioTask::GetGameplayTasksComponent(const UGameplayTask& Task) const
{
	return ((&Task == this || ChildTasks.Contains(&Task)) ? UGameplayTask::GetGameplayTasksComponent() : nullptr);
}

AActor* UAsyncScenarioTask::GetGameplayTaskOwner(const UGameplayTask* Task) const
{
	return ((Task == this || ChildTasks.Contains(Task)) ? GetAvatarActor() : nullptr);
}

void UAsyncScenarioTask::OnGameplayTaskInitialized(UGameplayTask& Task)
{
	const FName NewInstanceName = Task.GetInstanceName();
	if (Task.IsA<UAsyncScenarioTask>())
	{
		const bool bIsDuplicateTaskName =
			ChildTasks.ContainsByPredicate([NewInstanceName](const UGameplayTask* Itr) { return (Itr->GetInstanceName() == NewInstanceName); });
		UE_CLOG(bIsDuplicateTaskName, LogScenario, Warning, TEXT("Duplicate TaskName \"%s\" for AsyncScenarioTask \"%s\", child task of \"%s\""),
			*NewInstanceName.ToString(), *Task.GetName(), *GetName());
	}

	ChildTasks.AddUnique(&Task);
}

void UAsyncScenarioTask::OnGameplayTaskActivated(UGameplayTask& Task)
{
	ActiveChildTasks.Add(&Task);
}

void UAsyncScenarioTask::OnGameplayTaskDeactivated(UGameplayTask& Task)
{
	ActiveChildTasks.Remove(&Task);

	const UAsyncScenarioTask* ScenarioTask = Cast<UAsyncScenarioTask>(&Task);
	if (!IsValid(ScenarioTask) || !ScenarioTask->bKeepTaskRunningAfterCompletion)
	{
		ChildTasks.Remove(Task);
	}
}

void UAsyncScenarioTask::Activate()
{
	Super::Activate();

	TaskStartTime = GetWorld()->GetTimeSeconds();
	TaskEndTime.Reset();

	ReceiveStart();
}

void UAsyncScenarioTask::OnDestroy(bool bHasOwnerFinished)
{
	if (const UWorld* World = GetWorld())
	{
		TaskEndTime = World->GetTimeSeconds();
	}

	Cleanup();

	TaskState = EGameplayTaskState::Finished;

	if (TaskOwner.IsValid())
	{
		TaskOwner->OnGameplayTaskDeactivated(*this);
	}

	if (!bKeepTaskRunningAfterCompletion || bHasOwnerFinished)
	{
		MarkAsGarbage();
	}
}

void UAsyncScenarioTask::Cleanup()
{
	while (ActiveChildTasks.Num() > 0)
	{
		if (UGameplayTask* Task = ActiveChildTasks.Pop())
		{
			Task->TaskOwnerEnded();
		}
	}

	ActiveChildTasks.Reset();
	ChildTasks.Reset();

	if (UWorld* World = GetWorld(); IsValid(World))
	{
		World->GetLatentActionManager().RemoveActionsForObject(this);
		World->GetTimerManager().ClearAllTimersForObject(this);
	}
}

void UAsyncScenarioTask::Complete(FGameplayTag Result)
{
	CompletionResult = Result;

	ReceiveComplete(Result);
	OnCompleted.Broadcast(Result);

	EndTask();
}

void UAsyncScenarioTask::ReceiveStart_Implementation() {}
void UAsyncScenarioTask::ReceiveComplete_Implementation(const FGameplayTag& Result) {}
