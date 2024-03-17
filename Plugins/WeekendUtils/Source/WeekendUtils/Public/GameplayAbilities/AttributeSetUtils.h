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
#include "AbilitySystemComponent.h"

#include "AttributeSetUtils.generated.h"

class UAttributeSet;
class UDataTable;

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName)           \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)               \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)               \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

///////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct WEEKENDUTILS_API FAttributeSetConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, NoClear)
	TSubclassOf<UAttributeSet> AttributeSet = nullptr;

	UPROPERTY(EditAnywhere, NoClear)
	TObjectPtr<UDataTable> InitializationData = nullptr;
};
