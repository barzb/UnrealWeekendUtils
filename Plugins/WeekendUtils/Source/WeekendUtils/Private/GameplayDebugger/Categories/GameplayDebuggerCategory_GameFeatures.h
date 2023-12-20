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
#include "GameplayDebugger/GameplayDebuggerUtils.h"
#include "GameplayDebuggerCategory.h"

/**
 * Gameplay Debugger Category "GameFeatures" shows internal information about
 * the current state of Game Feature plugins. See @UGameFeaturesSubsystem.
 */
class FGameplayDebuggerCategory_GameFeatures : public FGameplayDebuggerCategory
{
public:
	FGameplayDebuggerCategory_GameFeatures();
	GENERATE_DEBUGGER_CATEGORY(GameFeatures);

	// - FGameplayDebuggerCategory
	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	// --
};

#endif