///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AbilitySystemComponent.h"
#include "AttributeSetUtils.h"
#include "CoreMinimal.h"
#include "InputActionAbilityTypes.h"

#include "EnhancedAbilitySystemComponent.generated.h"

/**
 * Ability System Component that allows binding abilities directly to input actions.
 * @require EnhancedInputSystem
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class WEEKENDUTILS_API UEnhancedAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UEnhancedAbilitySystemComponent();

	///////////////////////////////////////////////////////////////////////////////////////
	/// OVERRIDES

	// - UAbilitySystemComponent
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
	virtual void BindToInputComponent(UInputComponent* InputComponent) override;
	virtual void BindAbilityActivationToInputComponent(UInputComponent* InputComponent, FGameplayAbilityInputBinds BindInfo) override final;
	// - UObject
	virtual void BeginDestroy() override;
	// --

	///////////////////////////////////////////////////////////////////////////////////////

	TArray<FGameplayAbilitySpecHandle> GetAbilitiesBoundToInputAction(const UInputAction* InputAction) const;
	TArray<FGameplayAbilitySpecHandle> GetAbilitiesBoundToInputId(int32 InputId) const;
	bool HasAnyAbilitiesBoundToInputAction(const UInputAction* InputAction) const;
	bool HasAnyAbilitiesBoundToInputId(int32 InputId) const;

protected:
	///////////////////////////////////////////////////////////////////////////////////////
	/// CLASS CONFIG

	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (ShowOnlyInnerProperties, TitleProperty = "DisplayName"))
	TArray<FInputActionBindableAbility> DefaultAbilities = {};

	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (ShowOnlyInnerProperties, TitleProperty = "DisplayName"))
	TArray<FAttributeSetConfig> DefaultAttributes = {};

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects = {};

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TObjectPtr<const UInputAction> InputActionForGenericConfirm = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TObjectPtr<const UInputAction> InputActionForGenericCancel = nullptr;

	///////////////////////////////////////////////////////////////////////////////////////
	/// RUNTIME STATE

	UPROPERTY(Transient)
	TArray<TObjectPtr<UAttributeSet>> AddedAttributeSets = {};

	///////////////////////////////////////////////////////////////////////////////////////

	virtual void GiveDefaultAbilities();
	virtual void InitDefaultAttributeSets();
	virtual void ApplyDefaultEffects();

	virtual void HandleGameplayTagsChanged(const FGameplayTag ChangedTag, int32 NewTagCount);

private:
	bool bIsAlreadyBoundToInputComponent = false;
};
