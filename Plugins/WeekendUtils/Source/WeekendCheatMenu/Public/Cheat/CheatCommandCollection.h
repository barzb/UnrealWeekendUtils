///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "CheatMenuSettings.h"

class ICheatMenuAction;

namespace Cheats
{
	class WEEKENDCHEATMENU_API FCheatCommandCollection
	{
	public:
		FCheatCommandCollection();
		FCheatCommandCollection(const FCheatMenuCategorySettings& InCheatMenuSettings);

		void AddCheat(ICheatMenuAction* CheatMenuAction);
		void RemoveCheat(ICheatMenuAction* CheatMenuAction);
		TArray<ICheatMenuAction*> GetRegisteredCheatMenuActions() const { return RegisteredCheatMenuActions; }

		bool ShowInCheatMenu() const { return CheatMenuSettings.IsSet(); }
		FCheatMenuCategorySettings GetCheatMenuSettings() const { return *CheatMenuSettings; }

	private:
		TOptional<FCheatMenuCategorySettings> CheatMenuSettings;
		TArray<ICheatMenuAction*> RegisteredCheatMenuActions;
	};

	WEEKENDCHEATMENU_API TArray<FCheatCommandCollection*>& GetAllCollections();

	/** Utility for #DEFINE_CHEAT_COLLECTION macro. @see CheatCommand.h */
	inline FCheatMenuCategorySettings AsCheatMenuTab(FName TabName) { return FCheatMenuCategorySettings().Tab(TabName); }
}
