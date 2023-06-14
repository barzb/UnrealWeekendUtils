///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz in cooperation with Nine Worlds Studios GmbH.
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See accompanying file LICENSE.
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Cheat/CheatMenuExtension.h"

namespace Cheats
{
	TArray<ICheatMenuExtension*> ICheatMenuExtension::AllExtensions;

	ICheatMenuExtension::ICheatMenuExtension(const FCheatCommandCollection& InCollection) :
		CheatMenuSettings(InCollection.GetCheatMenuSettings())
	{
		AllExtensions.AddUnique(this);
	}

	FCheatMenuExtension_CVar::FCheatMenuExtension_CVar(const FCheatCommandCollection& InCollection, FAutoConsoleObject* InCVar, const ECVarStyle& InDisplayStyle, const FString& InDisplayName) :
		ICheatMenuExtension(InCollection),
		CVar(InCVar), DisplayStyle(InDisplayStyle), DisplayName(InDisplayName)
	{
	}

	TSharedRef<SWidget> FCheatMenuExtension_CVar::Construct() const
	{
		
	}
}
