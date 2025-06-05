///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "GameplayAbilities/EnhancedAbilitySystemComponent.h"

#include "EnhancedInputComponent.h"


UEnhancedAbilitySystemComponent::UEnhancedAbilitySystemComponent()
{
	RegisterGenericGameplayTagEvent().AddUObject(this, &ThisClass::HandleGameplayTagsChanged);
}

void UEnhancedAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	if (!AbilityActorInfo.IsValid())
		return;

	if (const UWorld* World = InOwnerActor->GetWorld(); !World || !World->IsGameWorld())
		return;

	if (!AbilityActorInfo->AnimInstance.IsValid())
	{
		AbilityActorInfo->AnimInstance = AbilityActorInfo->GetAnimInstance();
	}

	InitDefaultAttributeSets();
	ApplyDefaultEffects();
	GiveDefaultAbilities();
}

void UEnhancedAbilitySystemComponent::BindToInputComponent(UInputComponent* InputComponent)
{
	if (!ensure(!bIsAlreadyBoundToInputComponent))
		return; // Avoid duplicate bindings.
	bIsAlreadyBoundToInputComponent = true;

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!ensure(IsValid(EnhancedInputComponent)))
		return;

	// Bind every possible input action that is currently known to the asset registry:
	for (const UInputAction* InputAction : TObjectRange<UInputAction>())
	{
		const int32 InputId = FInputActionBindableAbility::CreateInputIdForInputAction(*InputAction);
		EnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Triggered, this, &UAbilitySystemComponent::AbilityLocalInputPressed, InputId);
		EnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &UAbilitySystemComponent::AbilityLocalInputReleased, InputId);
	}

	// Bind generic ability input actions:
	EnhancedInputComponent->BindAction(InputActionForGenericConfirm, ETriggerEvent::Triggered, this, &UAbilitySystemComponent::LocalInputConfirm);
	EnhancedInputComponent->BindAction(InputActionForGenericCancel, ETriggerEvent::Triggered, this, &UAbilitySystemComponent::LocalInputCancel);
}

void UEnhancedAbilitySystemComponent::BindAbilityActivationToInputComponent(UInputComponent* InputComponent, FGameplayAbilityInputBinds BindInfo)
{
	BindToInputComponent(InputComponent);
}

void UEnhancedAbilitySystemComponent::BeginDestroy()
{
	Super::BeginDestroy();
}

TArray<FGameplayAbilitySpecHandle> UEnhancedAbilitySystemComponent::GetAbilitiesBoundToInputAction(const UInputAction* InputAction) const
{
	return GetAbilitiesBoundToInputId(FInputActionBindableAbility::CreateInputIdForInputAction(*InputAction));
}

TArray<FGameplayAbilitySpecHandle> UEnhancedAbilitySystemComponent::GetAbilitiesBoundToInputId(int32 InputId) const
{
	TArray<FGameplayAbilitySpecHandle> Result;
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (AbilitySpec.InputID == InputId)
		{
			Result.Add(AbilitySpec.Handle);
		}
	}
	return Result;
}

bool UEnhancedAbilitySystemComponent::HasAnyAbilitiesBoundToInputAction(const UInputAction* InputAction) const
{
	return HasAnyAbilitiesBoundToInputId(FInputActionBindableAbility::CreateInputIdForInputAction(*InputAction));
}

bool UEnhancedAbilitySystemComponent::HasAnyAbilitiesBoundToInputId(int32 InputId) const
{
	return (GetAbilitiesBoundToInputId(InputId).Num() > 0);
}

void UEnhancedAbilitySystemComponent::GiveDefaultAbilities()
{
	for (const FInputActionBindableAbility& Config : DefaultAbilities)
	{
		if (!Config.AbilityClass)
			continue;

		GiveAbility(FGameplayAbilitySpec(Config.AbilityClass, Config.AbilityLevel, Config.CreateInputId()));
	}
}

void UEnhancedAbilitySystemComponent::InitDefaultAttributeSets()
{
	for (const FAttributeSetConfig& Config : DefaultAttributes)
	{
		if (!Config.AttributeSet)
			continue;

		UAttributeSet* AttributeSet = NewObject<UAttributeSet>(this, Config.AttributeSet);
		if (Config.InitializationData)
		{
			AttributeSet->InitFromMetaDataTable(Config.InitializationData);
		}

		AddedAttributeSets.Add(AttributeSet);
		AddAttributeSetSubobject(AttributeSet);
	}
}

void UEnhancedAbilitySystemComponent::ApplyDefaultEffects()
{
	FGameplayEffectContextHandle EffectContext = MakeEffectContext();
	EffectContext.AddSourceObject(AbilityActorInfo->OwnerActor.Get());

	for (const TSubclassOf<UGameplayEffect>& Effect : DefaultEffects)
	{
		if (!Effect)
			continue;

		const UGameplayEffect* GameplayEffect = Effect->GetDefaultObject<UGameplayEffect>();
		ApplyGameplayEffectToSelf(GameplayEffect, 0, EffectContext);
	}
}

void UEnhancedAbilitySystemComponent::HandleGameplayTagsChanged(const FGameplayTag ChangedTag, int32 NewTagCount)
{
	//#todo
}
