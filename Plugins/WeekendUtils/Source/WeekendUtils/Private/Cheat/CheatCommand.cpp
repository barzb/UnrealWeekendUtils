///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz in cooperation with Nine Worlds Studios GmbH.
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See accompanying file LICENSE.
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Cheat/CheatCommand.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

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
	if (LogInvalidity(InWorld, "Cannot execute with invalid world!"))
		return;

	// Set temporary members for internal Execute() call of child class:
	World = InWorld;
	Args = InArgs;
	NextArgsIndex = 0;

	if (InArgs.IsEmpty())
	{
		LogInfo(FString::Printf(TEXT("Execute")));
	}
	else
	{
		LogInfo(FString::Printf(TEXT("Execute with Args: %s"), *FString::Join(InArgs, TEXT(", "))));
	}

	Execute();
	OnAfterExecuted.Broadcast(*this, InWorld, InArgs);

	// Clear temporary members after execution:
	World = nullptr;
	Args.Empty();
	NextArgsIndex = 0;
}

FString ICheatCommand::FArgumentInfo::ToShortString() const
{
	return FString::Printf(TEXT("[%s]: %s"), *Name, *LexToString(Style));
}

FString ICheatCommand::FArgumentInfo::ToString() const
{
	if (Description.IsEmpty())
		return ToShortString();
	return FString::Printf(TEXT("[%s]: %s - %s"), *Name, *LexToString(Style), *Description);
}

FString ICheatCommand::GetFullDescription() const
{
	if (ArgumentsInfo.IsEmpty())
		return CommandInfo;

	// => "FunctionDescription | [Number] IntArgument1: IntArgument1Description | [True/False] BoolArgument2: BoolArgument2Description"
	return CommandInfo + " | " + FString::JoinBy(ArgumentsInfo, TEXT(" | "), &FArgumentInfo::ToString);
}

TArray<ICheatCommand*> ICheatCommand::GetVariants(const UWorld* InWorld) const
{
	TArray<ICheatCommand*> Variants;
	if (GetRuntimeVariantsFunc.IsSet())
	{
		(*GetRuntimeVariantsFunc)(InWorld, OUT Variants);
	}
	return Variants;
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

APlayerController* ICheatCommand::GetLocalPlayerController() const
{
	return GetWorld()->GetFirstPlayerController();
}

APlayerState* ICheatCommand::GetLocalPlayerState() const
{
	const APlayerController* LocalPlayerController = GetLocalPlayerController();
	return IsValid(LocalPlayerController) ? LocalPlayerController->GetPlayerState<APlayerState>() : nullptr;
}

APawn* ICheatCommand::GetLocalPlayerPawn() const
{
	const APlayerController* LocalPlayerController = GetLocalPlayerController();
	return IsValid(LocalPlayerController) ? LocalPlayerController->GetPawn() : nullptr;
}

bool ICheatCommand::LogInvalidity(const UObject* Object, FString ErrorMessage) const
{
	if (!IsValid(Object))
	{
		LogError(ErrorMessage);
		return true;
	}
	return false;
}

void ICheatCommand::ExecuteOtherCheat(ICheatCommand& OtherCheatCommand)
{
	OtherCheatCommand.Execute(Args, World);
}

UObject* ICheatCommand::GetOrCreateSharedContextObjectForWorld()
{
	static AActor* ContextObject = nullptr;
	if (!IsValid(ContextObject) || ContextObject->GetWorld() != GetWorld())
	{
		if (IsValid(ContextObject))
		{
			ContextObject->Destroy();
		}
		FActorSpawnParameters Params;
		Params.Name = FName("CheatCommand_Context");
		Params.ObjectFlags = RF_Standalone|RF_Transient;
#if WITH_EDITOR
		Params.bHideFromSceneOutliner = true;
#endif

		ContextObject = GetWorld()->SpawnActor<AActor>(Params);
	}
	return ContextObject;
}

FString ICheatCommand::GetDefaultMissingArgumentError() const
{
	return "Missing argument: " + (ArgumentsInfo.IsValidIndex(NextArgsIndex) ? ArgumentsInfo[NextArgsIndex].Name : FString::FromInt(NextArgsIndex));
}
