///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
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
	namespace Cheats::CollectionName                                                  \
	{                                                                                 \
		FCheatCommandCollection Collection = FCheatCommandCollection(__VA_ARGS__);    \
	}                                                                                 \
	namespace Cheats::CollectionName

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
	///////////////////////////////////////////////////////////////////////////////////////
	/// Nested Types:

	using EArgumentStyle = Cheats::EVariableStyle::Type;
	struct FArgumentInfo
	{
		FString Name = "";
		FString Description = "";
		EArgumentStyle Style = EArgumentStyle::Text;
		FString ToShortString() const;
		FString ToString() const;
	};

	/** Temporary data container passed to the constructor. See macro definitions above for usage info. */
	struct FDescriber
	{
		/** Set the display name of the cheat command. @example: DisplayAs("Increase Score") */
		FDescriber& DisplayAs(FString InDisplayName);

		/** Describe the function of the cheat command. @example: DescribeCheat("Increases the player score by an amount.") */
		FDescriber& DescribeCheat(FString InDescription);

		/** Describe an argument of the cheat command. @example: DescribeArgument<int32>("Amount", "How much score to add. Default: 100") */
		template <typename ArgumentType>
		FDescriber& DescribeArgument(FString InArgumentName, FString InDescription = "");

		TOptional<FString> NameForDisplay = {};
		TOptional<FString> FunctionDescription = {};
		TArray<FArgumentInfo> ArgumentDescriptions;
	};

	///////////////////////////////////////////////////////////////////////////////////////
	/// Events:

	/** Event fired after the cheat command printed a log message. */
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnLogMessage, const ICheatCommand&, ELogVerbosity::Type, const FString& /*Message*/);
	FOnLogMessage OnLogMessage;

	/** Event fired after the cheat was executed. */
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnAfterExecuted, const ICheatCommand&, UWorld*, TArray<FString> /*Args*/);
	FOnAfterExecuted OnAfterExecuted;

	///////////////////////////////////////////////////////////////////////////////////////
	/// Execution:

	/** Executes this cheat command with given arguments and world, as if invoked by the console command. */
	void Execute(const TArray<FString>& InArgs, UWorld* InWorld);

	FORCEINLINE const FString& GetCommandName() const { return CommandName; }
	FORCEINLINE const FString& GetDisplayName() const { return DisplayName; }
	FORCEINLINE const FString& GetCommandInfo() const { return CommandInfo; }
	FORCEINLINE const TArray<FArgumentInfo>& GetArgumentsInfo() const { return ArgumentsInfo; }

	/** @returns the command info and arguments info in a single string. */
	FString GetFullDescription() const;

	///////////////////////////////////////////////////////////////////////////////////////
	/// Variants:

	/** @returns all runtime-generated variants of this cheat command, if existing. */
	TArray<ICheatCommand*> GetVariants(const UWorld* InWorld) const;
	TOptional<TFunction<void(const UWorld*, TArray<ICheatCommand*>&)>> GetRuntimeVariantsFunc = {};

protected:
	///////////////////////////////////////////////////////////////////////////////////////

	ICheatCommand(const FString& InCommandName, const FDescriber& InDescriber);
	virtual ~ICheatCommand() = default;
	virtual void Execute() = 0;

	///////////////////////////////////////////////////////////////////////////////////////
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

	/** @returns whether the passed object is invalid (=true) or valid (=false) with the possibility to log an error in case of invalidity. */
	bool LogInvalidity(const UObject* Object, FString ErrorMessage) const;

	FORCEINLINE UWorld* GetWorld() const { return World; }
	FORCEINLINE FString GetNextStringArgument() const { return Args.IsValidIndex(NextArgsIndex) ? Args[NextArgsIndex++] : ""; }

	/** @returns the local player controller (or nullptr) in the cheat execution world. */
	template <typename T> T* GetLocalPlayerController() const { return Cast<T>(GetLocalPlayerController()); }
	APlayerController* GetLocalPlayerController() const;

	/** @returns the local player state (or nullptr) in the cheat execution world. */
	template <typename T> T* GetLocalPlayerState() const { return Cast<T>(GetLocalPlayerState()); }
	APlayerState* GetLocalPlayerState() const;

	/** @returns the local player pawn (or nullptr) in the cheat execution world. */
	template <typename T> T* GetLocalPlayerPawn() const { return Cast<T>(GetLocalPlayerPawn()); }
	APawn* GetLocalPlayerPawn() const;

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

	/** Executes another cheat command with the same arguments and world as this cheat was executed with. */
	void ExecuteOtherCheat(ICheatCommand& OtherCheatCommand);

	/** @returns a statically allocated UObject that is shared between all cheats, but only exists inside of the current world. */
	UObject* GetOrCreateSharedContextObjectForWorld();

private:
	TObjectPtr<UWorld> World = nullptr;
	TArray<FString> Args;
	mutable int32 NextArgsIndex = 0;
	FString GetDefaultMissingArgumentError() const;

	/// (!) Members above are only valid inside the derived Execute() call.
	///////////////////////////////////////////////////////////////////////////////////////

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
	const int32 ArgsIdx = NextArgsIndex++;
	return (Args.IsValidIndex(ArgsIdx) && !Args[ArgsIdx].IsEmpty()) ? Args[ArgsIdx] : Fallback;
}

template <typename ArgumentType, typename>
ArgumentType ICheatCommand::GetNextArgumentOr(ArgumentType Fallback) const
{
	// (i) T must have a global LexTryParseString() override.
	ArgumentType TypedArg;
	if (!LexTryParseString(OUT TypedArg, *GetNextStringArgument()))
	{
		TypedArg = Fallback;
	}
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
	// (i) T == FString.
	const int32 ArgsIdx = NextArgsIndex++;
	if (!Args.IsValidIndex(ArgsIdx) || Args[ArgsIdx].IsEmpty())
	{
		const FString Error = CustomError.IsEmpty() ? GetDefaultMissingArgumentError() : CustomError;
		LogError(FString::Printf(TEXT("Missing argument: %s"), *Error));
		return {};
	}
	return Args[ArgsIdx];
}

template <typename ArgumentType, typename>
TOptional<ArgumentType> ICheatCommand::GetNextArgumentOrError(FString CustomError) const
{
	// (i) T must have a global LexTryParseString() override.
	ArgumentType TypedArg;
	if (!LexTryParseString(OUT TypedArg, *GetNextStringArgument()))
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

inline ICheatCommand::FDescriber& ICheatCommand::FDescriber::DisplayAs(FString InDisplayName)
{
	NameForDisplay = InDisplayName;
	return *this;
}

inline ICheatCommand::FDescriber& ICheatCommand::FDescriber::DescribeCheat(FString InDescription)
{
	FunctionDescription = InDescription;
	return *this;
}

template <typename ArgumentType>
ICheatCommand::FDescriber& ICheatCommand::FDescriber::DescribeArgument(FString InArgumentName, FString InDescription)
{
	FArgumentInfo& ArgumentInfo = ArgumentDescriptions.AddDefaulted_GetRef();
	ArgumentInfo.Name = InArgumentName;
	ArgumentInfo.Description = InDescription;
	ArgumentInfo.Style = Cheats::EVariableStyle::FromType<ArgumentType>();
	return *this;
}
