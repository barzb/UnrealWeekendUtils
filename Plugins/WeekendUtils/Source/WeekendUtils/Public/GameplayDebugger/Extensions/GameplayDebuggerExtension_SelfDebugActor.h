///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz in cooperation with Nine Worlds Studios GmbH.
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See accompanying file LICENSE.
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