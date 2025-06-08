///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"

#include "InputActionAbilityTypes.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;
struct FGameplayAbilitySpec;
struct FGameplayAbilitySpecHandle;

USTRUCT(BlueprintType)
struct WEEKENDUTILS_API FInputActionBindableAbility
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (InlineEditConditionToggle), Category = "Weekend Utils|Abilities|Input Action")
	bool bBindToInputAction = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bBindToInputAction"), Category = "Weekend Utils|Abilities|Input Action")
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weekend Utils|Abilities|Input Action")
	FText DisplayName = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weekend Utils|Abilities|Input Action")
	TSubclassOf<UGameplayAbility> AbilityClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weekend Utils|Abilities|Input Action")
	int32 AbilityLevel = 0;

	///////////////////////////////////////////////////////////////////////////////////////

	/** @returns the cached input action assigned to a runtime-unique (but not save-game unique) input ID. */
	static const UInputAction* FindCachedInputActionForInputId(int32 InputId);

	/** @returns a runtime-unique (but not save-game unique) ID for an input action, to be used for input binding abilities. */
	static int32 CreateInputIdForInputAction(const UInputAction& InputAction);

	/** @returns the runtime-unique (but not save-game unique) ID of the configured InputAction, to be used for input binding the configured ability. */
	int32 CreateInputId() const;

	/** @returns an ability spec for the ability configured in this config, for the given ability system. */
	FGameplayAbilitySpec BuildAbilitySpec(UObject* SourceObject = nullptr) const;

	/** @returns the handle to the ability granted to the given ability system. */
	FGameplayAbilitySpecHandle GiveAbilityTo(UAbilitySystemComponent& AbilitySystem, UObject* SourceObject = nullptr) const;
};
