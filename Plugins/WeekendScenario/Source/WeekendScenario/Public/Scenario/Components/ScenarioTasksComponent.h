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
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "GameplayTasksComponent.h"

#include "ScenarioTasksComponent.generated.h"

struct FCurrentSaveGame;

DECLARE_MULTICAST_DELEGATE(FOnScenarioTagsChanged)

/**
 * #todo-docs
 */
UCLASS()
class WEEKENDSCENARIO_API UScenarioTasksComponent : public UGameplayTasksComponent,
													public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	FOnScenarioTagsChanged& OnGameplayTagsChanged() { return OnGameplayTagsChangedEvent; }

	//#todo-scenario ConfirmTag (transient -> savegame)
	virtual void AddGameplayTag(const FGameplayTag& GameplayTag);
	virtual void AddGameplayTags(const FGameplayTagContainer& GameplayTags);
	virtual void RemoveGameplayTag(const FGameplayTag& GameplayTag);
	virtual void RemoveGameplayTags(const FGameplayTagContainer& GameplayTags);

	// - IGameplayTagAssetInterface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& OutTagContainer) const override;
	// --

	virtual void WriteToSaveGame(const FCurrentSaveGame& SaveGame);
	virtual void RestoreFromSaveGame(const FCurrentSaveGame& SaveGame);

protected:
	FGameplayTagContainer GameplayTagContainer;
	FOnScenarioTagsChanged OnGameplayTagsChangedEvent;
};
