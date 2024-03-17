///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "GameplayAbilities/EnhancedGameplayAbility.h"

#include "Engine/InputDelegateBinding.h"

void UEnhancedGameplayAbility::BindToInputComponent(UInputComponent* InputComponent)
{
	if (!InputComponent)
	{
		InputComponent = GetOwningActorFromActorInfo()->FindComponentByClass<UInputComponent>();
	}

	if (ensure(IsInstantiated()))
	{
		UInputDelegateBinding::BindInputDelegates(GetClass(), InputComponent, this);
	}
}

void UEnhancedGameplayAbility::UnbindFromInputComponent(UInputComponent* InputComponent)
{
	if (!InputComponent)
	{
		InputComponent = GetOwningActorFromActorInfo()->FindComponentByClass<UInputComponent>();
		if (!InputComponent)
			return;
	}

	if (ensure(IsInstantiated()))
	{
		InputComponent->ClearBindingsForObject(this);
	}
}