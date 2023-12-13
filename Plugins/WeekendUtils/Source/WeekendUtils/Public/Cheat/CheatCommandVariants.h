///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz in cooperation with Aesir Interactive GmbH.
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See accompanying file LICENSE.
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CheatCommand.h"

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
/// 	} #todo
///////////////////////////////////////////////////////////////////////////////////////

#define BEGIN_DEFINE_CHEAT_VARIANTS(CheatName)                                        \
			class F##CheatName##CommandVariants : public TArray<FCheatCommandVariant> \
			{                                                                         \
				TObjectPtr<const UWorld> PrivateTempWorld;                            \
				const UWorld* GetWorld() const { return PrivateTempWorld; }           \
			public:                                                                   \
				F##CheatName##CommandVariants()                                       \
				{                                                                     \
					CheatName.GetVariantsFunc =                                       \
					[this](const UWorld* World, TArray<ICheatCommand*>& OutVariants ) \
					{                                                                 \
						Empty(); PrivateTempWorld = World;

#define DEFINE_CHEAT_VARIANT(CheatName, CommandName, DisplayName, ...)                \
						OutVariants.Add(&Add_GetRef(FCheatCommandVariant(             \
								&CheatName, CommandName,                              \
								ICheatCommand::FDescriber()                           \
								.DisplayAs(DisplayName)                               \
								.DescribeCheat(CheatName.GetCommandInfo()),           \
								{__VA_ARGS__}                                         \
						)));

#define END_DEFINE_CHEAT_VARIANTS(CheatName) PrivateTempWorld = nullptr;              \
			}; } } CheatName##Variants;

///////////////////////////////////////////////////////////////////////////////////////

/**
 * todo
 */
class FCheatCommandVariant final : public ICheatCommand
{
public:
	FCheatCommandVariant(ICheatCommand* InOriginalCommand, const FString& InCommandName, const FDescriber& InDescriber, const TArray<FString>& InOverrideArgs) :
		ICheatCommand(InCommandName, InDescriber),
		OriginalCommand(InOriginalCommand),
		OverrideArgs(InOverrideArgs)
	{
	}

	virtual void Execute() override
	{
		OriginalCommand->Execute(OverrideArgs, GetWorld());
	}

	ICheatCommand* OriginalCommand;
	TArray<FString> OverrideArgs;
};
