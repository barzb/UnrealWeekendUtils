///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2024 by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendScenario UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Scenario/ScenarioService.h"

#include "Scenario/Scenario.h"

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
	return ScenarioTasksComponent->OnGameplayTagsChanged();
}

void UScenarioService::StartService()
{
	SpawnScenarioServiceActor();
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
}

void UScenarioService::SpawnScenarioServiceActor()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = MakeUniqueObjectName(this, AActor::StaticClass(), "ScenarioServiceActor");
	SpawnParams.bNoFail = true;
	ScenarioServiceActor = GetWorld()->SpawnActor<AActor>(SpawnParams);
	ScenarioServiceActor->bAlwaysRelevant = true;

	ScenarioTasksComponent = NewObject<UScenarioTasksComponent>(ScenarioServiceActor);
	ScenarioServiceActor->AddOwnedComponent(ScenarioTasksComponent);
	ScenarioTasksComponent->RegisterComponent();
}

void UScenarioService::HandleScenarioCompletion(UScenario* CompletedScenario)
{
	RunningScenarios.Remove(CompletedScenario);
}
