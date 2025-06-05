///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "GameService/WorldServiceConfigBase.h"

namespace Internal
{
	static TMap<FString, TSubclassOf<UWorldServiceConfigBase>> GConfigClassesByWorldName;
}

void UWorldServiceConfigBase::RegisterForWorldsWhoseNamesContain(const FString& PartOfWorldName)
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		Internal::GConfigClassesByWorldName.Add(PartOfWorldName, GetClass());
	}
}

const UWorldServiceConfigBase* UWorldServiceConfigBase::FindConfigForWorld(const UWorld& World)
{
	const FString FullWorldName = World.GetName();
	TArray<FString> WorldNameStrings;
	Internal::GConfigClassesByWorldName.GenerateKeyArray(OUT WorldNameStrings);
	for (const FString& String : WorldNameStrings)
	{
		if (FullWorldName.Contains(String, ESearchCase::IgnoreCase))
			return Internal::GConfigClassesByWorldName[String]->GetDefaultObject<UWorldServiceConfigBase>();
	}

	return nullptr; // No config found for world name.
}
