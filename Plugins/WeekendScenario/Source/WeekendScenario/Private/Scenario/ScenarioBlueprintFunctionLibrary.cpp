///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendScenario UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Scenario/ScenarioBlueprintFunctionLibrary.h"

#include "GameService/GameServiceLocator.h"
#include "GameplayTagAssetInterface.h"
#include "Scenario/ScenarioService.h"

namespace
{
	TOptional<FGameplayTagContainer> GetScenarioTags()
	{
		const TWeakObjectPtr<const UScenarioService> ScenarioService = UGameServiceLocator::FindServiceAsWeakPtr<UScenarioService>();
		if (!ScenarioService.IsValid())
			return {};

		const IGameplayTagAssetInterface* ScenarioTagsProvider = ScenarioService->GetScenarioTagsProvider();
		if (!ScenarioTagsProvider)
			return {};

		FGameplayTagContainer Tags;
		ScenarioTagsProvider->GetOwnedGameplayTags(OUT Tags);
		return Tags;
	}
}

ECommonValidity UScenarioBlueprintFunctionLibrary::GetGameScenarioTags(FGameplayTagContainer& OutTags)
{
	TOptional<FGameplayTagContainer> ScenarioTags = GetScenarioTags();
	if (!ScenarioTags.IsSet())
		return ECommonValidity::Invalid;

	OutTags.Reset();
	OutTags.AppendTags(ScenarioTags.GetValue());
	return ECommonValidity::Valid;
}
