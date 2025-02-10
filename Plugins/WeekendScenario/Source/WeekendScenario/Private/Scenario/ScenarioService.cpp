///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendScenario UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Scenario/ScenarioService.h"

#include "SaveGame/SaveGameService.h"
#include "Scenario/Scenario.h"
#include "Settings/ScenarioProjectSettings.h"

UScenario& UScenarioService::RunScenario(const TSubclassOf<UScenario>& ScenarioClass, FName TaskName)
{
	check(ScenarioClass != nullptr);
	check(IsValid(ScenarioTasksComponent));

	UScenario* NewScenario = UScenario::NewScenarioTask<UScenario>(ScenarioTasksComponent, ScenarioClass, TaskName);
	RunningScenarios.Add(NewScenario);

	//NewScenario->OnCompleted.AddDynamic(this, &ThisClass::HandleScenarioCompletion, NewScenario);
	NewScenario->ReadyForActivation();

	return *NewScenario;
}

IGameplayTagAssetInterface* UScenarioService::GetScenarioTagsProvider() const
{
	return ScenarioTasksComponent;
}

FOnScenarioTagsChanged& UScenarioService::OnScenarioTagsChanged()
{
	check(ScenarioTasksComponent);
	return ScenarioTasksComponent->OnGameplayTagsChanged();
}

void UScenarioService::StartRestorableService(const FCurrentSaveGame& SaveGame)
{
	SpawnScenarioServiceActor();

	RestoreFromSaveGame(SaveGame);
}

void UScenarioService::WriteToSaveGame(const FCurrentSaveGame& InOutSaveGame)
{
	// Write states of running scenario tasks into the service state:
	CurrentState->ScenarioTaskStates.Empty();
	TArray<const UAsyncScenarioTask*> TasksToProcess = TArray<const UAsyncScenarioTask*>(RunningScenarios);
	while (TasksToProcess.Num() > 0)
	{
		const UAsyncScenarioTask* RunningTask = TasksToProcess.Pop();
		TasksToProcess.Append(RunningTask->GetActiveChildTasks());

		CurrentState->ScenarioTaskStates.Add(RunningTask->GetPathName(), RunningTask->GetActiveTaskData());
	}

	// Mediate writing the state of the scenario tasks component:
	ScenarioTasksComponent->WriteToSaveGame(InOutSaveGame);
}

void UScenarioService::RestoreFromSaveGame(const FCurrentSaveGame& SaveGame)
{
	CurrentState = &UModularSaveGame::SummonModule<UScenarioServiceState>(*this, ServiceStateClass);
	ScenarioTasksComponent->RestoreFromSaveGame(SaveGame);
}

void UScenarioService::ShutdownService()
{
	for (UScenario* Scenario : RunningScenarios)
	{
		Scenario->OnCompleted.RemoveAll(this);
		Scenario->ExternalCancel();
	}
	RunningScenarios.Empty();

	if (IsValid(ScenarioServiceActor))
	{
		ScenarioServiceActor->Destroy();
	}

	Super::ShutdownService();
}

FActiveScenarioTaskData UScenarioService::SummonScenarioTaskData(const UAsyncScenarioTask& ScenarioTask) const
{
	return CurrentState->ScenarioTaskStates.FindOrAdd(ScenarioTask.GetPathName());
}

void UScenarioService::SpawnScenarioServiceActor()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = MakeUniqueObjectName(this, AActor::StaticClass(), "ScenarioServiceActor");
	SpawnParams.bNoFail = true;
	ScenarioServiceActor = GetWorld()->SpawnActor<AActor>(SpawnParams);
	ScenarioServiceActor->bAlwaysRelevant = true;

	const TSubclassOf<UScenarioTasksComponent>& TasksComponentClass = GetDefault<UScenarioProjectSettings>()->ScenarioTasksComponentClass;
	ScenarioTasksComponent = NewObject<UScenarioTasksComponent>(ScenarioServiceActor, TasksComponentClass);
	ScenarioServiceActor->AddOwnedComponent(ScenarioTasksComponent);
	ScenarioTasksComponent->RegisterComponent();
}

void UScenarioService::HandleScenarioCompletion(UScenario* CompletedScenario)
{
	RunningScenarios.Remove(CompletedScenario);
}
