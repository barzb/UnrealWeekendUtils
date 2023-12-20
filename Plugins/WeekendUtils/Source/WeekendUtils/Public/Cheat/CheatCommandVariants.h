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
/// The following macros are intended to define preset ("variant") cheat commands more
/// conveniently. Variants are created and collected non-statically at runtime as soon
/// as the cheat menu opens. This allows for dynamic cheats in different environments.
///
/// @example:
/// 	DEFINE_CHEAT_COLLECTION(ExampleCheats, AsCheatMenuTab("Tab1").Section("Misc"))
/// 	{
/// 		DEFINE_CHEAT_COMMAND(IncreaseScoreCheat, "Cheat.Score.Increase")
/// 		.DisplayAs("Increase Score")
/// 		.DescribeCheat("Increases the player score by a certain amount.")
/// 		.DescribeArgument<int32>("Amount", "The additional score. Default: 100")
/// 		DEFINE_CHEAT_EXECUTE(IncreaseScoreCheat)
/// 		{
///				...
/// 		}
///
/// 		BEGIN_DEFINE_CHEAT_VARIANTS(IncreaseScoreCheat)
/// 			DEFINE_CHEAT_VARIANT("Cheat.Score.Increase.By100",
/// 				"+100 Score", IncreaseScoreCheat.GetCommandInfo(), {"100"})
/// 			DEFINE_CHEAT_VARIANT("Cheat.Score.Increase.By200",
/// 				"+200 Score", "Custom Description", {"200"})
/// 			if (GetWorld() && GetWorld()->IsPlayInEditor())
/// 			{
/// 				DEFINE_CHEAT_VARIANT("Cheat.Score.Increase.ForEditorOnly",
/// 					"+999 Score (PIE only)", "Only available in PIE", {"999"})
/// 			}
/// 		END_DEFINE_CHEAT_VARIANTS(IncreaseScoreCheat)
/// 	}
///////////////////////////////////////////////////////////////////////////////////////

#define BEGIN_DEFINE_CHEAT_VARIANTS(CheatName)                                        \
			class F##CheatName##CommandVariants : public TArray<FCheatCommandVariant> \
			{                                                                         \
				ICheatCommand* OriginalCheat;                                         \
				TObjectPtr<const UWorld> PrivateTempWorld;                            \
				const UWorld* GetWorld() const { return PrivateTempWorld; }           \
			public:                                                                   \
				F##CheatName##CommandVariants()                                       \
				{                                                                     \
					OriginalCheat = &CheatName;                                       \
					CheatName.GetRuntimeVariantsFunc =                                \
					[this](const UWorld* World, TArray<ICheatCommand*>& OutVariants)  \
					{                                                                 \
						Empty(); PrivateTempWorld = World;

#define DEFINE_CHEAT_VARIANT(CommandName, DisplayName, Description, ArgsArray)        \
						OutVariants.Add(&Add_GetRef(FCheatCommandVariant(             \
								OriginalCheat, CommandName,                           \
								ICheatCommand::FDescriber()                           \
								.DisplayAs(DisplayName)                               \
								.DescribeCheat(Description),                          \
								ArgsArray                                             \
						)));

#define END_DEFINE_CHEAT_VARIANTS(CheatName) PrivateTempWorld = nullptr;              \
			}; } } CheatName##Variants;

///////////////////////////////////////////////////////////////////////////////////////

/**
 * Proxy class for variants of another cheat command that predefine passed arguments at construction.
 * @remark: See macros defined above for how to use.
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

protected:
	ICheatCommand* OriginalCommand;
	TArray<FString> OverrideArgs;
};
