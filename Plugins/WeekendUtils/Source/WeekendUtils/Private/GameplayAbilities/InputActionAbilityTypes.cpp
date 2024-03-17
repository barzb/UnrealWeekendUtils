///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "GameplayAbilities/InputActionAbilityTypes.h"

#include "AbilitySystemComponent.h"

namespace
{
	TArray<TWeakObjectPtr<const UInputAction>> GAbilityBoundInputActions = {};
}

const UInputAction* FInputActionBindableAbility::FindCachedInputActionForInputId(int32 InputId)
{
	if (!GAbilityBoundInputActions.IsValidIndex(InputId))
		return nullptr;

	return GAbilityBoundInputActions[InputId].Get();
}

int32 FInputActionBindableAbility::CreateInputIdForInputAction(const UInputAction& InputAction)
{
	if (const int32 ExistingId = GAbilityBoundInputActions.IndexOfByKey(&InputAction); ExistingId != INDEX_NONE)
		return ExistingId;

	return GAbilityBoundInputActions.Add(&InputAction);
}

int32 FInputActionBindableAbility::CreateInputId() const
{
	return ((bBindToInputAction && InputAction) ? CreateInputIdForInputAction(*InputAction) : INDEX_NONE);
}

FGameplayAbilitySpec FInputActionBindableAbility::BuildAbilitySpec(UObject* SourceObject) const
{
	return FGameplayAbilitySpec{AbilityClass, AbilityLevel, CreateInputId(), SourceObject};
}

FGameplayAbilitySpecHandle FInputActionBindableAbility::GiveAbilityTo(UAbilitySystemComponent& AbilitySystem, UObject* SourceObject) const
{
	return AbilitySystem.GiveAbility(BuildAbilitySpec(SourceObject));
}
