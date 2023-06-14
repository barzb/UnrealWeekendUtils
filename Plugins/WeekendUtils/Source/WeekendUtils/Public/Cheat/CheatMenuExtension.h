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
#include "CheatCommand.h"
#include "CheatMenuSettings.h"
#include "CheatCommandCollection.h"

#define REGISTER_CHEAT_MENU_CVAR(CVarRef, DisplayStyle, DisplayName) \
	namespace { FCheatMenuExtension_CVar CheatMenuExtension_##CVarRef (Collection, &CVarRef, DisplayStyle, DisplayName); }

namespace Cheats
{
	using ECVarStyle = EVariableStyle::Type;
	class WEEKENDUTILS_API ICheatMenuExtension //#todo rename to ICheatMenuEntry?
	{
	public:
		static TArray<ICheatMenuExtension*> AllExtensions;

		virtual TSharedRef<SWidget> Construct() const = 0;

	protected:
		explicit ICheatMenuExtension(const FCheatCommandCollection& InCollection);
		~ICheatMenuExtension() = default;

	private:
		FCheatMenuCategorySettings CheatMenuSettings;
	};

	class WEEKENDUTILS_API FCheatMenuExtension_CVar : public ICheatMenuExtension
	{
	public:
		FCheatMenuExtension_CVar(const FCheatCommandCollection& InCollection, FAutoConsoleObject* InCVar, const ECVarStyle& InDisplayStyle, const FString& InDisplayName);
		virtual ~FCheatMenuExtension_CVar() = default;
		virtual TSharedRef<SWidget> Construct() const override;

	private:
		FAutoConsoleObject* CVar;
		ECVarStyle DisplayStyle;
		FString DisplayName;
	};
}

DEFINE_CHEAT_COLLECTION(BlaCollection)
{
	static TAutoConsoleVariable<bool> CVar_Blafu(
		TEXT("gdt.Category.Agents.ShowAttributeFragments"), true,
		TEXT("Enable to show agent attributes in the GameplayDebugger category: Agents"));

	REGISTER_CHEAT_MENU_CVAR(CVar_Blafu, ECVarStyle::Text, "Blafu");
}
