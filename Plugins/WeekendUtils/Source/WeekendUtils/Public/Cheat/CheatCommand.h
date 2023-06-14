///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz in cooperation with Nine Worlds Studios GmbH.
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See accompanying file LICENSE.
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CheatCommandCollection.h" // For use in macros.
#include "CheatMenuSettings.h"
#include "Engine/World.h"
#include "HAL/ConsoleManager.h"
#include "Logging/LogVerbosity.h"

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
	namespace Cheats::##CollectionName                                                \
	{                                                                                 \
		FCheatCommandCollection Collection = FCheatCommandCollection(__VA_ARGS__);    \
	}                                                                                 \
	namespace Cheats::##CollectionName

#define DEFINE_CHEAT_COMMAND(CheatName, CommandName)                                  \
			class F##CheatName##Command final : public ICheatCommand                  \
			{                                                                         \
			public:                                                                   \
				virtual void Execute() override;                                      \
				F##CheatName##Command() : ICheatCommand(CommandName, FDescriber()

#define DEFINE_CHEAT_EXECUTE(CheatName) ) { Collection.AddCheat(this); }              \
			} CheatName;                                                              \
			void F##CheatName##Command::Execute()

///////////////////////////////////////////////////////////////////////////////////////

WEEKENDUTILS_API DECLARE_LOG_CATEGORY_EXTERN(LogCheatCmd, Log, All);

/**
 * Abstract base class for cheat commands that can be executed from command line.
 * @remark: See macros defined above for how to use.
 */
class WEEKENDUTILS_API ICheatCommand
{
public:
	using EArgumentStyle = Cheats::EVariableStyle::Type;
	struct FArgumentInfo
	{
		FString Name = "";
		FString Description = "";
		EArgumentStyle Style = EArgumentStyle::Text;
	};

	/** Event fired after the cheat command printed a log message. */
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnLogMessage, const ICheatCommand&, ELogVerbosity::Type, const FString& /*Message*/);
	FOnLogMessage OnLogMessage;

	/** Event fired after the cheat was executed. */
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnAfterExecuted, const ICheatCommand&, UWorld*, TArray<FString> /*Args*/);
	FOnAfterExecuted OnAfterExecuted;

	/** Executes this cheat command with given arguments and world, as if invoked by the console command. */
	void Execute(const TArray<FString>& InArgs, UWorld* InWorld);

	FORCEINLINE const FString& GetCommandName() const { return CommandName; }
	FORCEINLINE const FString& GetDisplayName() const { return DisplayName; }
	FORCEINLINE const FString& GetCommandInfo() const { return CommandInfo; }
	FORCEINLINE const TArray<FArgumentInfo>& GetArgumentsInfo() const { return ArgumentsInfo; }

	/** @returns the command info and arguments info in a single string. */
	FString GetFullDescription() const;

protected:
	/** Temporary data container passed to the constructor. See macro definitions above for usage info. */
	struct FDescriber
	{
		/** Set the display name of the cheat command. @example: DisplayAs("Increase Score") */
		FDescriber& DisplayAs(FString DisplayName);

		/** Describe the function of the cheat command. @example: DescribeCheat("Increases the player score by an amount.") */
		FDescriber& DescribeCheat(FString Description);

		/** Describe an argument of the cheat command. @example: DescribeArgument<int32>("Amount", "How much score to add. Default: 100") */
		template <typename ArgumentType>
		FDescriber& DescribeArgument(FString ArgumentName, FString Description);

		TOptional<FString> NameForDisplay = {};
		TOptional<FString> FunctionDescription = {};
		TArray<FArgumentInfo> ArgumentDescriptions;
	};

	ICheatCommand(const FString& InCommandName, const FDescriber&);
	virtual ~ICheatCommand() = default;
	virtual void Execute() = 0;

	////////////////////////////////////////////////////////////////////////////////////
	/// (!) Members below are only valid inside the derived Execute() call:

	/** Sends a log message with verbosity 'Display' to the logfile, console and any listeners (like the CheatMenu). */
	void LogInfo(const FString& Message) const;

	/** Sends a log message with verbosity 'Warning' to the logfile, console and any listeners (like the CheatMenu). */
	void LogWarning(const FString& Message) const;

	/** Sends a log message with verbosity 'Error' to the logfile, console and any listeners (like the CheatMenu). */
	void LogError(const FString& Message) const;

	/** Sends a log message with verbosity 'Verbose' to the logfile, console and any listeners (like the CheatMenu). */
	void LogVerbose(const FString& Message) const;

	/** Sends a log message with verbosity 'VeryVerbose' to the logfile, console and any listeners (like the CheatMenu). */
	void LogVeryVerbose(const FString& Message) const;

	FORCEINLINE UWorld* GetWorld() const { return World; }
	FORCEINLINE FString GetNextStringArgument() const { return Args.IsValidIndex(NextArgsIndex) ? Args[NextArgsIndex++] : ""; }

	/** @returns the next argument in the argument list - or given fallback value if the argument does not exist. */
	template <typename ArgumentType = FString, typename = typename TEnableIf<TIsConstructible<FString, ArgumentType>::Value>::Type, typename T2 = void>
	ArgumentType GetNextArgumentOr(ArgumentType Fallback) const;

	/** @returns the next argument in the argument list - or given fallback value if the argument does not exist. */
	template <typename ArgumentType, typename = typename TEnableIf<TIsConstructible<FString, ArgumentType>::Value == false>::Type>
	ArgumentType GetNextArgumentOr(ArgumentType Fallback) const;

	/**
	 * @returns the next argument in the argument list - or given fallback value if the argument does not exist.
	 * @Parser must take a const FString& and return a TOptional<ArgumentType>
	 */
	template <typename ArgumentType, typename Parser>
	ArgumentType GetNextArgumentOr(ArgumentType Fallback, Parser CustomParser) const;

	/** @returns the next argument in the argument list - or logs an error if the argument does not exist. */
	template <typename ArgumentType = FString, typename = typename TEnableIf<TIsConstructible<FString, ArgumentType>::Value>::Type, typename T = void>
	TOptional<ArgumentType> GetNextArgumentOrError(FString CustomError = "") const;

	/** @returns the next argument in the argument list - or logs an error if the argument does not exist. */
	template <typename ArgumentType, typename = typename TEnableIf<TIsConstructible<FString, ArgumentType>::Value == false>::Type>
	TOptional<ArgumentType> GetNextArgumentOrError(FString CustomError = "") const;

	/**
	 * @returns the next argument in the argument list - or logs an error if the argument does not exist.
	 * @Parser must take a const FString& and return a TOptional<ArgumentType>
	 */
	template <typename ArgumentType, typename Parser>
	TOptional<ArgumentType> GetNextArgumentOrError(FString Error, Parser CustomParser) const;


private:
	TObjectPtr<UWorld> World = nullptr;
	TArray<FString> Args;
	mutable int32 NextArgsIndex = 0;
	FString GetDefaultMissingArgumentError() const;

	/// (!) Members above are only valid inside the derived Execute() call.
	////////////////////////////////////////////////////////////////////////////////////

	const FString CommandName;
	const FString DisplayName;
	const FString CommandInfo;
	const TArray<FArgumentInfo> ArgumentsInfo;
	FAutoConsoleCommandWithWorldAndArgs ConsoleCommand;
};

///////////////////////////////////////////////////////////////////////////////////////
/// Inlines for ICheatCommand:

template <typename ArgumentType, typename, typename T>
ArgumentType ICheatCommand::GetNextArgumentOr(ArgumentType Fallback) const
{
	// (i) T == FString.
	return Args.IsValidIndex(NextArgsIndex) ? Args[NextArgsIndex++] : Fallback;
}

template <typename ArgumentType, typename>
ArgumentType ICheatCommand::GetNextArgumentOr(ArgumentType Fallback) const
{
	// (i) T must have a global LexFromString() override.
	ArgumentType TypedArg = Fallback;
	LexFromString(OUT TypedArg, *GetNextStringArgument());
	return TypedArg;
}

template <typename ArgumentType, typename Parser>
ArgumentType ICheatCommand::GetNextArgumentOr(ArgumentType Fallback, Parser CustomParser) const
{
	return Invoke(CustomParser, GetNextStringArgument()).Get(Fallback);
}

template <typename ArgumentType, typename, typename T>
TOptional<ArgumentType> ICheatCommand::GetNextArgumentOrError(FString CustomError) const
{
	if (!Args.IsValidIndex(NextArgsIndex))
	{
		const FString Error = CustomError.IsEmpty() ? GetDefaultMissingArgumentError() : CustomError;
		LogError(FString::Printf(TEXT("Missing argument: %s"), *Error));
		return {};
	}
	return Args[NextArgsIndex++];
}

template <typename ArgumentType, typename>
TOptional<ArgumentType> ICheatCommand::GetNextArgumentOrError(FString CustomError) const
{
	ArgumentType TypedArg;
	if (!TryLexFromString(OUT TypedArg, *GetNextStringArgument()))
	{
		const FString Error = CustomError.IsEmpty() ? GetDefaultMissingArgumentError() : CustomError;
		LogError(FString::Printf(TEXT("Missing or wrong argument: %s"), *Error));
		return {};
	}
	return TypedArg;
}

template <typename ArgumentType, typename Parser>
TOptional<ArgumentType> ICheatCommand::GetNextArgumentOrError(FString Error, Parser CustomParser) const
{
	TOptional<ArgumentType> ParsedArg = Invoke(CustomParser, GetNextStringArgument());
	if (!ParsedArg.IsSet())
	{
		LogError(FString::Printf(TEXT("Missing or wrong argument: %s"), *Error));
		return {};
	}
	return ParsedArg;
}

///////////////////////////////////////////////////////////////////////////////////////
/// Inlines for ICheatCommand::FDescriber:

inline ICheatCommand::FDescriber& ICheatCommand::FDescriber::DisplayAs(FString DisplayName)
{
	NameForDisplay = DisplayName;
	return *this;
}

inline ICheatCommand::FDescriber& ICheatCommand::FDescriber::DescribeCheat(FString Description)
{
	FunctionDescription = Description;
	return *this;
}

template <typename ArgumentType>
ICheatCommand::FDescriber& ICheatCommand::FDescriber::DescribeArgument(FString ArgumentName, FString Description)
{
	FArgumentInfo& ArgumentInfo = ArgumentDescriptions.AddDefaulted_GetRef();
	ArgumentInfo.Name = ArgumentName;
	ArgumentInfo.Description = Description;
	ArgumentInfo.Style = Cheats::EVariableStyle::FromType<ArgumentType>();
	return *this;
}
