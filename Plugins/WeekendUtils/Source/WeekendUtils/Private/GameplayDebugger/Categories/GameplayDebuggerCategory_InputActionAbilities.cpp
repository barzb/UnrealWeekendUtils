///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////


#if WITH_GAMEPLAY_DEBUGGER

#include "GameplayDebuggerCategory_InputActionAbilities.h"

#include "AbilitySystemGlobals.h"
#include "GameplayAbilities/EnhancedAbilitySystemComponent.h"

FGameplayDebuggerCategory_InputActionAbilities::FGameplayDebuggerCategory_InputActionAbilities()
{
	bShowOnlyWithDebugActor = false;
}

void FGameplayDebuggerCategory_InputActionAbilities::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	if (!OwnerPC)
		return;

	const APlayerController* LocalPC = OwnerPC->GetWorld()->GetFirstPlayerController();
	const UAbilitySystemComponent* AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(LocalPC->GetPawn());
	CollectInputBoundAbilitiesData(Cast<UEnhancedAbilitySystemComponent>(AbilitySystem));
}

void FGameplayDebuggerCategory_InputActionAbilities::CollectInputBoundAbilitiesData(const UEnhancedAbilitySystemComponent* AbilitySystem)
{
	if (!AbilitySystem)
		return;

	for (const UInputAction* InputAction : TObjectRange<UInputAction>())
	{
		const int32 InputId = FInputActionBindableAbility::CreateInputIdForInputAction(*InputAction);
		TArray<FGameplayAbilitySpecHandle> BoundAbilitySpecHandles = AbilitySystem->GetAbilitiesBoundToInputId(InputId);

		for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : BoundAbilitySpecHandles)
		{
			const FGameplayAbilitySpec* AbilitySpec = AbilitySystem->FindAbilitySpecFromHandle(AbilitySpecHandle);
			if (!AbilitySpec)
				continue;

			const FColor AbilityNameTextColor = (AbilitySpec->Ability ? FColor::Green : FColor::Red);
			AddTextLine(FString::Printf(TEXT("{white}%03d [{yellow}%s{white}] {%s}%s"),
				InputId, *GetNameSafe(InputAction),
				*AbilityNameTextColor.ToString(),
				*GetNameSafe(AbilitySpec->Ability ? AbilitySpec->Ability->GetClass() : nullptr)));
		}
	}
}

#endif
