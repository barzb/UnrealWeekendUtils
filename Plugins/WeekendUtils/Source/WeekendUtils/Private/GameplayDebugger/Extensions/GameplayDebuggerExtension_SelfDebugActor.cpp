///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#if WITH_GAMEPLAY_DEBUGGER

#include "GameplayDebugger/Extensions/GameplayDebuggerExtension_SelfDebugActor.h"

#include "GameplayDebuggerCategoryReplicator.h"
#include "GameFramework/PlayerController.h"

FGameplayDebuggerExtension_SelfDebugActor::FGameplayDebuggerExtension_SelfDebugActor()
{
	const FGameplayDebuggerInputHandlerConfig KeyConfig(TEXT("Trigger"), EKeys::Home.GetFName());
	bHasInputBinding = BindKeyPress(KeyConfig, this, &FGameplayDebuggerExtension_SelfDebugActor::SelectSelfAsDebugActor);
}

FString FGameplayDebuggerExtension_SelfDebugActor::GetDescription() const
{
	if (!bHasInputBinding)
		return FString();

	const bool bIsAlreadySelfSelected =
		   GetReplicator() != nullptr
		&& GetReplicator()->GetDebugActor() != nullptr
		&& GetReplicator()->GetDebugActor() == LastSelectedSelfActor.Get();

	return FString::Printf(TEXT("{%s}%s:{%s}Select Self"),
		*FGameplayDebuggerCanvasStrings::ColorNameInput,
		*GetInputHandlerDescription(0),
		bIsAlreadySelfSelected ? *FGameplayDebuggerCanvasStrings::ColorNameDisabled : *FGameplayDebuggerCanvasStrings::ColorNameEnabled);
}

void FGameplayDebuggerExtension_SelfDebugActor::SelectSelfAsDebugActor()
{
	UWorld* World = GetWorldFromReplicator();
	if (!World)
		return;

	APlayerController* LocalPlayerController = World->GetFirstPlayerController();
	if (!LocalPlayerController)
		return;

	APawn* LocallyControlledPawn = LocalPlayerController->GetPawn();
	if (!LocallyControlledPawn)
		return;

	GetReplicator()->SetDebugActor(LocallyControlledPawn);
	LastSelectedSelfActor = MakeWeakObjectPtr(LocallyControlledPawn);
}

#endif
