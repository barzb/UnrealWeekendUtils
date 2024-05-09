///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

///////////////////////////////////////////////////////////////////////////////////////
/// The following macros are intended to define cheat commands more conveniently.
/// Commands will be created in cheat collections in the "Cheats::" namespace.
/// @example:
/// 	DEFINE_CHEAT_COLLECTION(ExampleCheats, AsCheatMenuTab("Tab1").Section("Misc"))
/// 	{
/// 		DEFINE_CHEAT_COMMAND(IncreaseScoreCheat, "Cheat.Score.Increase")
/// 		.DisplayAs("Increase Score")
/// 		.DescribeCheat("Increases the player score by a certain amount.")
/// 		.DescribeArgument<int32>("Amount", "The additional score. Default: 100")
/// 		DEFINE_CHEAT_EXECUTE(IncreaseScoreCheat)
/// 		{
/// 			const int32 Amount = GetNextArgumentOr<int32>(100);
/// 			UScoreSystem::AddScore(Amount);
/// 			LogInfo("Score was increased by " + FString::FromInt(Amount));
/// 		}
///
/// 		// ..Define more cheat commands
/// 	}
///////////////////////////////////////////////////////////////////////////////////////

#define DEFINE_CHEAT_COLLECTION(CollectionName, ...)                                  \
	namespace Cheats::CollectionName                                                  \
	{                                                                                 \
		FCheatCommandCollection Collection = FCheatCommandCollection(__VA_ARGS__);    \
	}                                                                                 \
	namespace Cheats::CollectionName

#define DEFINE_CHEAT_COMMAND(CheatName, CommandName)                                  \
	class F##CheatName##Command final : public ICheatCommand                          \
	{                                                                                 \
	public:                                                                           \
		virtual void Execute() override;                                              \
		virtual ~F##CheatName##Command() override { Collection.RemoveCheat(this); }   \
		F##CheatName##Command() : ICheatCommand(CommandName, FDescriber()

#define DEFINE_CHEAT_MENU_ACTION(CheatName)                                           \
	class F##CheatName##Command final : public ICheatMenuAction                       \
	{                                                                                 \
	public:                                                                           \
		virtual void Execute() override;                                              \
		virtual ~F##CheatName##Command() override { Collection.RemoveCheat(this); }   \
		F##CheatName##Command() : ICheatMenuAction(#CheatName, FDescriber()

#define DEFINE_CHEAT_EXECUTE(CheatName) ) { Collection.AddCheat(this); }              \
	} CheatName;                                                                      \
	void F##CheatName##Command::Execute()

#define EXECUTE_AS_CHEAT_VARIANT(CheatName, VariantOfName, ...)                       \
	DEFINE_CHEAT_EXECUTE(CheatName)                                                   \
	{                                                                                 \
		VariantOfName.ExecuteWithArgs(TArray<FString>__VA_ARGS__, GetWorld());        \
	}

#define DEFINE_CHEAT_VARIANT(VariantOfName, Variant, DisplayName, ...)                \
	DEFINE_CHEAT_MENU_ACTION(VariantOfName##Variant)                                  \
	.DisplayAs(DisplayName)                                                           \
	EXECUTE_AS_CHEAT_VARIANT(VariantOfName##Variant, VariantOfName, __VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////////////

#include "CheatCommandCollection.h"
#include "CheatMenuAction.h"
#include "HAL/ConsoleManager.h"

/**
 * Base class for cheat commands that can be executed from command line.
 * @remark: See macros defined above for how to use.
 */
class WEEKENDUTILS_API ICheatCommand : public ICheatMenuAction
{
protected:
	ICheatCommand(const FString& InCommandName, const FDescriber& InDescriber) :
		ICheatMenuAction(InCommandName, InDescriber),
		ConsoleCommand(*InCommandName, *GetFullDescription(),
			FConsoleCommandWithWorldAndArgsDelegate::CreateRaw(this, &ICheatCommand::ExecuteWithArgs), ECVF_Cheat)
	{
	}

private:
	FAutoConsoleCommandWithWorldAndArgs ConsoleCommand;
};
