///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2024 by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendScenario UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Components/ScenarioTasksComponent.h"
#include "CoreMinimal.h"
#include "GameService/GameServiceBase.h"
#include "Scenario/Scenario.h"

#include "ScenarioService.generated.h"

class AActor;
class IGameplayTagAssetInterface;

/**
 * #todo-docs
 *
 * It is recommended to run a startup scenario in GameMode::InitGame.
 * If the previous map has some sort of scenario selection, the selected Scenario
 * could be passed as travel option and then received in InitGame(Options).
 */
UCLASS()
class WEEKENDSCENARIO_API UScenarioService : public UGameServiceBase
{
	GENERATED_BODY()

public:
	UScenario& RunScenario(const TSubclassOf<UScenario>& ScenarioClass, FName TaskName = NAME_None);

	FORCEINLINE UScenarioTasksComponent* GetScenarioTasksComponent() const { return ScenarioTasksComponent; }
	IGameplayTagAssetInterface* GetScenarioTagsProvider() const;
	FOnScenarioTagsChanged& OnScenarioTagsChanged();

	// - UGameServiceBase
	virtual void StartService() override;
	virtual void ShutdownService() override;
	// --

protected:
	UPROPERTY()
	TObjectPtr<AActor> ScenarioServiceActor = nullptr;

	UPROPERTY()
	TObjectPtr<UScenarioTasksComponent> ScenarioTasksComponent = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<UScenario>> RunningScenarios = {};

	void SpawnScenarioServiceActor();

	virtual void HandleScenarioCompletion(UScenario* CompletedScenario);
};
