///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz in cooperation with Nine Worlds Studios GmbH.
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See accompanying file LICENSE.
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "CheatMenuSettings.h"

class ICheatCommand;

namespace Cheats
{
	class WEEKENDUTILS_API FCheatCommandCollection
	{
	public:
		FCheatCommandCollection();
		FCheatCommandCollection(const FCheatMenuCategorySettings& InCheatMenuSettings);

		void AddCheat(ICheatCommand* CheatCommand);
		TArray<ICheatCommand*> GetRegisteredCheatCommands() const { return RegisteredCheatCommands; }

		bool ShowInCheatMenu() const { return CheatMenuSettings.IsSet(); }
		FCheatMenuCategorySettings GetCheatMenuSettings() const { return *CheatMenuSettings; }

		static TArray<FCheatCommandCollection*> AllCollections;

	private:
		TOptional<FCheatMenuCategorySettings> CheatMenuSettings;
		TArray<ICheatCommand*> RegisteredCheatCommands;
	};

	/** Utility for #DEFINE_CHEAT_COLLECTION macro. @see CheatCommand.h */
	inline FCheatMenuCategorySettings AsCheatMenuTab(FName TabName) { return FCheatMenuCategorySettings().Tab(TabName); }
}
