﻿///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2024 by Benjamin Barz and contributors. See file: CREDITS.md
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

class FGameplayDebuggerExtension_ToggleUiVisibility : public FGameplayDebuggerExtension
{
public:
	FGameplayDebuggerExtension_ToggleUiVisibility();
	GENERATE_DEBUGGER_EXTENSION(ToggleUiVisibility);

	// - FGameplayDebuggerExtension
	virtual FString GetDescription() const override;
	// --

private:
	static UClass* GetTopLevelWidgetClass();
	void ToggleUiVisibility();

	bool bHasInputBinding = false;
	bool bTurnVisibleNext = true;
};

#endif