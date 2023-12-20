///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#if WITH_GAMEPLAY_DEBUGGER

#include "CoreMinimal.h"
#include "GameplayDebuggerExtension.h"
#include "GameplayDebugger/GameplayDebuggerUtils.h"

class FGameplayDebuggerExtension_SelfDebugActor : public FGameplayDebuggerExtension
{
public:
	FGameplayDebuggerExtension_SelfDebugActor();
	GENERATE_DEBUGGER_EXTENSION(SelfDebugActor);

	// - FGameplayDebuggerExtension
	virtual FString GetDescription() const override;
	// --

private:
	void SelectSelfAsDebugActor();

	bool bHasInputBinding = false;
	TWeakObjectPtr<AActor> LastSelectedSelfActor = nullptr;
};

#endif