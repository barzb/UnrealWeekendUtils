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
#include "Abilities/GameplayAbility.h"

#include "EnhancedGameplayAbility.generated.h"

/**
 * Gameplay ability base class that allows binding to enhanced input events.
 */
UCLASS(Blueprintable, BlueprintType)
class WEEKENDUTILS_API UEnhancedGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	/** Enables blueprint (enhanced) input bindings to trigger inside of the ability graph. Instanced abilities only. */
	UFUNCTION(BlueprintCallable)
	virtual void BindToInputComponent(UInputComponent* InputComponent = nullptr);

	/** Disables blueprint (enhanced) input bindings to trigger inside of the ability graph. Instanced abilities only. */
	UFUNCTION(BlueprintCallable)
	virtual void UnbindFromInputComponent(UInputComponent* InputComponent = nullptr);
};
