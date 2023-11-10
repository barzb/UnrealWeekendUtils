///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz in cooperation with Aesir Interactive GmbH.
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See accompanying file LICENSE.
///
///////////////////////////////////////////////////////////////////////////////////////

#if WITH_GAMEPLAY_DEBUGGER

#include "GameplayDebuggerCategory_GameFeatures.h"

#include "GameFeaturesSubsystem.h"
#include "GameFeatureTypes.h"
#include "Interfaces/IPluginManager.h"

namespace
{
	FString GameFeatureStateToColoredString(const EGameFeaturePluginState& State)
	{
		switch (State)
		{
			case EGameFeaturePluginState::Registered:	return "{cyan}Registered";
			case EGameFeaturePluginState::Installed:	return "{red}Installed";
			case EGameFeaturePluginState::Loaded:		return "{yellow}Loaded";
			case EGameFeaturePluginState::Active:		return "{green}Active";
			default:									return "{grey}" + UE::GameFeatures::ToString(State);
		}
	}
}

FGameplayDebuggerCategory_GameFeatures::FGameplayDebuggerCategory_GameFeatures()
{
	bShowOnlyWithDebugActor = false;
}

void FGameplayDebuggerCategory_GameFeatures::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	TArray<FString> TextLines;
	const UGameFeaturesSubsystem& GameFeatures = UGameFeaturesSubsystem::Get();
	for (const TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetEnabledPlugins())
	{
		FString PluginUrl;
		if (!GameFeatures.GetPluginURLByName(Plugin->GetName(), OUT PluginUrl))
			continue;

		const EGameFeaturePluginState CurrentState = GameFeatures.GetPluginState(PluginUrl);
		TextLines.Add("{white}- " + Plugin->GetName() + " <" + GameFeatureStateToColoredString(CurrentState) + "{white}>");
	}
	TextLines.Sort();

	AddTextLine("{yellow}## Game Features ##");
	for (const FString& TextLine : TextLines)
	{
		AddTextLine(TextLine);
	}
}

#endif
