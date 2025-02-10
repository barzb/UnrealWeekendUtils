///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendScenario UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Scenario/Components/ScenarioTasksComponent.h"

void UScenarioTasksComponent::AddGameplayTag(const FGameplayTag& GameplayTag)
{
	unimplemented();
}

void UScenarioTasksComponent::AddGameplayTags(const FGameplayTagContainer& GameplayTags)
{
	unimplemented();
}

void UScenarioTasksComponent::RemoveGameplayTag(const FGameplayTag& GameplayTag)
{
	unimplemented();
}

void UScenarioTasksComponent::RemoveGameplayTags(const FGameplayTagContainer& GameplayTags)
{
	unimplemented();
}

void UScenarioTasksComponent::GetOwnedGameplayTags(FGameplayTagContainer& OutTagContainer) const
{
	OutTagContainer.Reset();
	OutTagContainer.AppendTags(GameplayTagContainer);
}

void UScenarioTasksComponent::WriteToSaveGame(const FCurrentSaveGame& SaveGame)
{
}

void UScenarioTasksComponent::RestoreFromSaveGame(const FCurrentSaveGame& SaveGame)
{
}
