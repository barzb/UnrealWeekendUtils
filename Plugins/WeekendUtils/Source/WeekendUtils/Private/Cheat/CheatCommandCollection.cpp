///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz in cooperation with Nine Worlds Studios GmbH.
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See accompanying file LICENSE.
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Cheat/CheatCommandCollection.h"

#include "Cheat/CheatCommand.h"

namespace Cheats
{
	TArray<FCheatCommandCollection*> FCheatCommandCollection::AllCollections;

	FCheatCommandCollection::FCheatCommandCollection()
	{
		AllCollections.AddUnique(this);
	}

	FCheatCommandCollection::FCheatCommandCollection(const FCheatMenuCategorySettings& InCheatMenuSettings) :
		CheatMenuSettings(InCheatMenuSettings)
	{
		AllCollections.AddUnique(this);
	}

	void FCheatCommandCollection::AddCheat(ICheatCommand* CheatCommand)
	{
		RegisteredCheatCommands.AddUnique(CheatCommand);
	}
}
