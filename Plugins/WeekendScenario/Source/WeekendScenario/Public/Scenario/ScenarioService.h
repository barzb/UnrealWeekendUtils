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
#include "Components/ScenarioTasksComponent.h"
#include "GameService/RestorableGameServiceBase.h"
#include "Scenario/Scenario.h"
#include "Scenario/Data/ScenarioServiceState.h"

#include "ScenarioService.generated.h"

class AActor;
class IGameplayTagAssetInterface;
class USaveGameService;

/**
 * #todo-docs
 *
 * It is recommended to run a startup scenario in GameMode::InitGame.
 * If the previous map has some sort of scenario selection, the selected Scenario
 * could be passed as travel option and then received in InitGame(Options).
 */
UCLASS(BlueprintType)
class WEEKENDSCENARIO_API UScenarioService : public URestorableGameServiceBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	UScenario* RunScenario(TSubclassOf<UScenario> ScenarioClass, FName TaskName = NAME_None);

	UScenarioTasksComponent* GetScenarioTasksComponent() const { return ScenarioTasksComponent; }
	IGameplayTagAssetInterface* GetScenarioTagsProvider() const;
	FOnScenarioTagsChanged& OnScenarioTagsChanged();

	// - URestorableGameServiceBase
	virtual void StartRestorableService(const FCurrentSaveGame& SaveGame) override;
	virtual void WriteToSaveGame(const FCurrentSaveGame& InOutSaveGame) override;
	virtual void RestoreFromSaveGame(const FCurrentSaveGame& SaveGame) override;
	virtual void ShutdownService() override;
	// --

	//#todo-scenario
	FActiveScenarioTaskData SummonScenarioTaskData(const UAsyncScenarioTask& ScenarioTask) const;

	UScenario* GetFirstRunningScenario() const;

protected:
	///////////////////////////////////////////////////////////////////////////////////////
	/// CLASS CONFIG

	UPROPERTY()
	TSubclassOf<UScenarioServiceState> ServiceStateClass = UScenarioServiceState::StaticClass();

	///////////////////////////////////////////////////////////////////////////////////////
	/// RUNTIME STATE

	UPROPERTY()
	TObjectPtr<UScenarioServiceState> CurrentState = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> ScenarioServiceActor = nullptr;

	UPROPERTY()
	TObjectPtr<UScenarioTasksComponent> ScenarioTasksComponent = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<UScenario>> RunningScenarios = {};

	///////////////////////////////////////////////////////////////////////////////////////

	void SpawnScenarioServiceActor();

	virtual void HandleScenarioCompletion(UScenario* CompletedScenario);
};
