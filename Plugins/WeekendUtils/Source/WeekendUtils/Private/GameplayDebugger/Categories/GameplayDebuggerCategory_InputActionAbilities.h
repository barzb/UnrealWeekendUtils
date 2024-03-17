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
#include "GameplayDebuggerCategory.h"

#include "GameplayDebugger/GameplayDebuggerUtils.h"

class UEnhancedAbilitySystemComponent;

class FGameplayDebuggerCategory_InputActionAbilities final : public FGameplayDebuggerCategory
{
public:
	FGameplayDebuggerCategory_InputActionAbilities();
	GENERATE_DEBUGGER_CATEGORY(InputActionAbilities);

	// - FGameplayDebuggerCategory
	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	// --

private:
	void CollectInputBoundAbilitiesData(const UEnhancedAbilitySystemComponent* AbilitySystem);
};

#endif // WITH_GAMEPLAY_DEBUGGER
