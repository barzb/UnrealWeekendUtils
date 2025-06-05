///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Cheat/CheatMenuAction.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

DEFINE_LOG_CATEGORY(LogCheatCmd);

ICheatMenuAction::ICheatMenuAction(const FString& InActionName, const FDescriber& InDescriber) :
	Name(InActionName),
	DisplayName(InDescriber.NameForDisplay.Get(InActionName)),
	ActionInfo(InDescriber.FunctionDescription.Get("")),
	ArgumentsInfo(InDescriber.ArgumentDescriptions)
{
}

void ICheatMenuAction::ExecuteWithArgs(const TArray<FString>& InArgs, UWorld* InWorld)
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

FString ICheatMenuAction::FArgumentInfo::ToShortString() const
{
	return FString::Printf(TEXT("[%s]: %s"), *Name, *LexToString(Style));
}

FString ICheatMenuAction::FArgumentInfo::ToString() const
{
	if (Description.IsEmpty())
		return ToShortString();
	return FString::Printf(TEXT("[%s]: %s - %s"), *Name, *LexToString(Style), *Description);
}

FString ICheatMenuAction::GetFullDescription() const
{
	if (ArgumentsInfo.IsEmpty())
		return ActionInfo;

	// => "FunctionDescription | [Number] IntArgument1: IntArgument1Description | [True/False] BoolArgument2: BoolArgument2Description"
	return ActionInfo + " | " + FString::JoinBy(ArgumentsInfo, TEXT(" | "), &FArgumentInfo::ToString);
}

void ICheatMenuAction::ExecuteOtherCheat(ICheatMenuAction& OtherCheatMenuAction)
{
	OtherCheatMenuAction.ExecuteWithArgs(Args, World);
}

void ICheatMenuAction::LogInfo(const FString& Message) const
{
	UE_LOG(LogCheatCmd, Display, TEXT("[%s] %s"), *Name, *Message);
	OnLogMessage.Broadcast(*this, ELogVerbosity::Display, Message);
}

void ICheatMenuAction::LogWarning(const FString& Message) const
{
	UE_LOG(LogCheatCmd, Warning, TEXT("[%s] %s"), *Name, *Message);
	OnLogMessage.Broadcast(*this, ELogVerbosity::Warning, Message);
}

void ICheatMenuAction::LogError(const FString& Message) const
{
	UE_LOG(LogCheatCmd, Error, TEXT("[%s] %s"), *Name, *Message);
	OnLogMessage.Broadcast(*this, ELogVerbosity::Error, Message);
}

void ICheatMenuAction::LogVerbose(const FString& Message) const
{
	UE_LOG(LogCheatCmd, Verbose, TEXT("[%s] %s"), *Name, *Message);
	OnLogMessage.Broadcast(*this, ELogVerbosity::Verbose, Message);
}

void ICheatMenuAction::LogVeryVerbose(const FString& Message) const
{
	UE_LOG(LogCheatCmd, VeryVerbose, TEXT("[%s] %s"), *Name, *Message);
	OnLogMessage.Broadcast(*this, ELogVerbosity::VeryVerbose, Message);
}

APlayerController* ICheatMenuAction::GetLocalPlayerController() const
{
	return GetWorld()->GetFirstPlayerController();
}

APlayerState* ICheatMenuAction::GetLocalPlayerState() const
{
	const APlayerController* LocalPlayerController = GetLocalPlayerController();
	return IsValid(LocalPlayerController) ? LocalPlayerController->GetPlayerState<APlayerState>() : nullptr;
}

APawn* ICheatMenuAction::GetLocalPlayerPawn() const
{
	const APlayerController* LocalPlayerController = GetLocalPlayerController();
	return IsValid(LocalPlayerController) ? LocalPlayerController->GetPawn() : nullptr;
}

bool ICheatMenuAction::LogInvalidity(const UObject* Object, const FString& ErrorMessage) const
{
	if (!IsValid(Object))
	{
		LogError(ErrorMessage);
		return true;
	}
	return false;
}

FString ICheatMenuAction::GetDefaultMissingArgumentError() const
{
	return "Missing argument: " + (ArgumentsInfo.IsValidIndex(NextArgsIndex) ? ArgumentsInfo[NextArgsIndex].Name : FString::FromInt(NextArgsIndex));
}

UObject* ICheatMenuAction::GetOrCreateSharedContextObjectForWorld()
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
