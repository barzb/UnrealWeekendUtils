///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#if WITH_GAMEPLAY_DEBUGGER

#include "CoreMinimal.h"
#include "GameplayDebugger/GameplayDebuggerUtils.h"
#include "GameplayDebuggerCategory.h"

/**
 * Gameplay Debugger Category "CyborgSlots" shows internal information about ... #todo-benni
 */
class FGameplayDebuggerCategory_CyborgSlots : public FGameplayDebuggerCategory
{
public:
	FGameplayDebuggerCategory_CyborgSlots();
	GENERATE_DEBUGGER_CATEGORY(CyborgSlots);

	// - FGameplayDebuggerCategory
	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	// --
};

#endif