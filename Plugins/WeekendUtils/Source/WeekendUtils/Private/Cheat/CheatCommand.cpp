///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz in cooperation with Nine Worlds Studios GmbH.
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See accompanying file LICENSE.
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Cheat/CheatCommand.h"

DEFINE_LOG_CATEGORY(LogCheatCmd);

ICheatCommand::ICheatCommand(const FString& InCommandName, const FDescriber& InDescriber) :
	CommandName(InCommandName),
	DisplayName(InDescriber.NameForDisplay.Get(InCommandName)),
	CommandInfo(InDescriber.FunctionDescription.Get("")),
	ArgumentsInfo(InDescriber.ArgumentDescriptions),
	ConsoleCommand(*InCommandName, *GetFullDescription(),
		FConsoleCommandWithWorldAndArgsDelegate::CreateRaw(this, &ICheatCommand::Execute), ECVF_Cheat)
{
}

void ICheatCommand::Execute(const TArray<FString>& InArgs, UWorld* InWorld)
{
	// Set temporary members for internal Execute() call of child class:
	World = InWorld;
	Args = InArgs;
	NextArgsIndex = 0;

	LogInfo(FString::Printf(TEXT("Execute with Args: %s"), *FString::Join(InArgs, TEXT(", "))));
	Execute();
	OnAfterExecuted.Broadcast(*this, InWorld, InArgs);

	// Clear temporary members after execution:
	World = nullptr;
	Args.Empty();
	NextArgsIndex = 0;
}

FString ICheatCommand::GetFullDescription() const
{
	if (ArgumentsInfo.IsEmpty())
		return CommandInfo;

	// => "FunctionDescription | [Number] IntArgument1: IntArgument1Description | [True/False] BoolArgument2: BoolArgument2Description"
	return CommandInfo + " | " + FString::JoinBy(ArgumentsInfo, TEXT(" | "), [](const FArgumentInfo& Argument)
	{
		return FString::Printf(TEXT("[%s] %s: %s"), *LexToString(Argument.Style), *Argument.Name, *Argument.Description);
	});
}

void ICheatCommand::LogInfo(const FString& Message) const
{
	UE_LOG(LogCheatCmd, Display, TEXT("[%s] %s"), *CommandName, *Message);
	OnLogMessage.Broadcast(*this, ELogVerbosity::Display, Message);
}

void ICheatCommand::LogWarning(const FString& Message) const
{
	UE_LOG(LogCheatCmd, Warning, TEXT("[%s] %s"), *CommandName, *Message);
	OnLogMessage.Broadcast(*this, ELogVerbosity::Warning, Message);
}

void ICheatCommand::LogError(const FString& Message) const
{
	UE_LOG(LogCheatCmd, Error, TEXT("[%s] %s"), *CommandName, *Message);
	OnLogMessage.Broadcast(*this, ELogVerbosity::Error, Message);
}

void ICheatCommand::LogVerbose(const FString& Message) const
{
	UE_LOG(LogCheatCmd, Verbose, TEXT("[%s] %s"), *CommandName, *Message);
	OnLogMessage.Broadcast(*this, ELogVerbosity::Verbose, Message);
}

void ICheatCommand::LogVeryVerbose(const FString& Message) const
{
	UE_LOG(LogCheatCmd, VeryVerbose, TEXT("[%s] %s"), *CommandName, *Message);
	OnLogMessage.Broadcast(*this, ELogVerbosity::VeryVerbose, Message);
}

FString ICheatCommand::GetDefaultMissingArgumentError() const
{
	return "Missing argument: " + (ArgumentsInfo.IsValidIndex(NextArgsIndex) ? ArgumentsInfo[NextArgsIndex].Name : FString::FromInt(NextArgsIndex));
}
