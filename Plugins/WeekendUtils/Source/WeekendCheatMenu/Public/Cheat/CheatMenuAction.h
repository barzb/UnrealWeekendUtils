///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CheatMenuSettings.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Logging/LogVerbosity.h"

WEEKENDCHEATMENU_API DECLARE_LOG_CATEGORY_EXTERN(LogCheatCmd, Log, All);

class WEEKENDCHEATMENU_API ICheatMenuAction
{
public:
	using EArgumentStyle = Cheats::EVariableStyle::Type;
	struct FArgumentInfo
	{
		FString Name = "";
		FString Description = "";
		EArgumentStyle Style = EArgumentStyle::Text;
		FString ToShortString() const;
		FString ToString() const;
		bool IsTextArgument() const;

		struct FOptionsSource // Only for EArgumentStyle::DropdownText
		{
			TOptional<TFunction<void(UWorld*, TArray<TSharedPtr<FString>>&)>> GetOptionsFunc = {};
			mutable TArray<TSharedPtr<FString>> Options = {};
			TArray<TSharedPtr<FString>>* GetOptions(UWorld* InWorld) const
			{
				if (GetOptionsFunc.IsSet())
				{
					Options.Empty();
					(*GetOptionsFunc)(InWorld, OUT Options);
				}
				return &Options;
			}
		} OptionsSource;
	};

	/** Temporary data container passed to the constructor. See macro definitions above for usage info. */
	struct FDescriber
	{
		/** Set the display name of the cheat command. @example: DisplayAs("Increase Score") */
		FDescriber& DisplayAs(const FString& InDisplayName);

		/** Describe the function of the cheat command. @example: DescribeCheat("Increases the player score by an amount.") */
		FDescriber& DescribeCheat(const FString& InDescription);

		/** Describe an argument of the cheat command. @example: DescribeArgument<int32>("Amount", "How much score to add. Default: 100") */
		template <typename ArgumentType>
		FDescriber& DescribeArgument(FString InArgumentName, FString InDescription = "");

		/** Describe an argument of the cheat command. @example: DescribeArgumentWithOptions(GGetAddItemOptionsFunc, "Item", "Which item to add") */
		template <typename Predicate = TFunction<void(UWorld*, TArray<TSharedPtr<FString>>&)>>
		FDescriber& DescribeArgumentWithOptions(Predicate GetOptionsFunc, FString InArgumentName, FString InDescription = "");

		TOptional<FString> NameForDisplay = {};
		TOptional<FString> FunctionDescription = {};
		TArray<FArgumentInfo> ArgumentDescriptions;
	};

	///////////////////////////////////////////////////////////////////////////////////////
	/// Events:

	/** Event fired after the cheat menu action printed a log message. */
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnLogMessage, const ICheatMenuAction&, ELogVerbosity::Type, const FString& /*Message*/);
	FOnLogMessage OnLogMessage;

	/** Event fired after the cheat menu action was executed. */
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnAfterExecuted, const ICheatMenuAction&, UWorld*, TArray<FString> /*Args*/);
	FOnAfterExecuted OnAfterExecuted;

	///////////////////////////////////////////////////////////////////////////////////////
	/// Execution:

	/** Executes this cheat command with given arguments and world, as if invoked by the console command. */
	void ExecuteWithArgs(const TArray<FString>& InArgs, UWorld* InWorld);

	FORCEINLINE const FString& GetName() const { return Name; }
	FORCEINLINE const FString& GetDisplayName() const { return DisplayName; }
	FORCEINLINE const FString& GetCommandInfo() const { return ActionInfo; }
	FORCEINLINE const TArray<FArgumentInfo>& GetArgumentsInfo() const { return ArgumentsInfo; }

	/** @returns the command info and arguments info in a single string. */
	FString GetFullDescription() const;

protected:
	///////////////////////////////////////////////////////////////////////////////////////
	ICheatMenuAction(const FString& InActionName, const FDescriber& InDescriber);
	virtual ~ICheatMenuAction() = default;
	virtual void Execute() = 0;

	/** Executes another cheat menu action with the same arguments and world as this cheat was executed with. */
	void ExecuteOtherCheat(ICheatMenuAction& OtherCheatMenuAction);

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
	bool LogInvalidity(const UObject* Object, const FString& ErrorMessage) const;

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

	FString GetDefaultMissingArgumentError() const;

	/** @returns a statically allocated UObject that is shared between all cheats, but only exists inside the current world. */
	UObject* GetOrCreateSharedContextObjectForWorld();

private:
	TObjectPtr<UWorld> World = nullptr;
	TArray<FString> Args;
	mutable int32 NextArgsIndex = 0;

	/// (!) Members above are only valid inside the derived Execute() call.
	///////////////////////////////////////////////////////////////////////////////////////

	const FString Name;
	const FString DisplayName;
	const FString ActionInfo;
	const TArray<FArgumentInfo> ArgumentsInfo;
};

///////////////////////////////////////////////////////////////////////////////////////
/// Inlines for ICheatMenuAction::FDescriber:

inline ICheatMenuAction::FDescriber& ICheatMenuAction::FDescriber::DisplayAs(const FString& InDisplayName)
{
	NameForDisplay = InDisplayName;
	return *this;
}

inline ICheatMenuAction::FDescriber& ICheatMenuAction::FDescriber::DescribeCheat(const FString& InDescription)
{
	FunctionDescription = InDescription;
	return *this;
}

template <typename ArgumentType>
ICheatMenuAction::FDescriber& ICheatMenuAction::FDescriber::DescribeArgument(FString InArgumentName, FString InDescription)
{
	FArgumentInfo& ArgumentInfo = ArgumentDescriptions.AddDefaulted_GetRef();
	ArgumentInfo.Name = InArgumentName;
	ArgumentInfo.Description = InDescription;
	ArgumentInfo.Style = Cheats::EVariableStyle::FromType<ArgumentType>();
	return *this;
}

template <typename Predicate>
ICheatMenuAction::FDescriber& ICheatMenuAction::FDescriber::DescribeArgumentWithOptions(Predicate GetOptionsFunc, FString InArgumentName, FString InDescription)
{
	FArgumentInfo& ArgumentInfo = ArgumentDescriptions.AddDefaulted_GetRef();
	ArgumentInfo.Name = InArgumentName;
	ArgumentInfo.Description = InDescription;
	ArgumentInfo.Style = Cheats::EVariableStyle::DropdownText;
	ArgumentInfo.OptionsSource.GetOptionsFunc = GetOptionsFunc;
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////
/// Inlines for ICheatMenuAction:

template <typename ArgumentType, typename, typename T>
ArgumentType ICheatMenuAction::GetNextArgumentOr(ArgumentType Fallback) const
{
	// (i) T == FString.
	const int32 ArgsIdx = NextArgsIndex++;
	return (Args.IsValidIndex(ArgsIdx) && !Args[ArgsIdx].IsEmpty()) ? Args[ArgsIdx] : Fallback;
}

template <typename ArgumentType, typename>
ArgumentType ICheatMenuAction::GetNextArgumentOr(ArgumentType Fallback) const
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
ArgumentType ICheatMenuAction::GetNextArgumentOr(ArgumentType Fallback, Parser CustomParser) const
{
	return Invoke(CustomParser, GetNextStringArgument()).Get(Fallback);
}

template <typename ArgumentType, typename, typename T>
TOptional<ArgumentType> ICheatMenuAction::GetNextArgumentOrError(FString CustomError) const
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
TOptional<ArgumentType> ICheatMenuAction::GetNextArgumentOrError(FString CustomError) const
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
TOptional<ArgumentType> ICheatMenuAction::GetNextArgumentOrError(FString Error, Parser CustomParser) const
{
	TOptional<ArgumentType> ParsedArg = Invoke(CustomParser, GetNextStringArgument());
	if (!ParsedArg.IsSet())
	{
		LogError(FString::Printf(TEXT("Missing or wrong argument: %s"), *Error));
		return {};
	}
	return ParsedArg;
}
