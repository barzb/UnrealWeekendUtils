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

#include "AttributeSetUtils.generated.h"

class UAttributeSet;
class UDataTable;

#define GAMEPLAYATTRIBUTE_BASE_VALUE_GETTER(PropertyName)		\
	FORCEINLINE float Get##PropertyName##BaseValue() const		\
	{															\
		return PropertyName.GetBaseValue();						\
	}

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName)			\
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)	\
	GAMEPLAYATTRIBUTE_BASE_VALUE_GETTER(PropertyName)			\
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)				\
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)				\
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

///////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct WEEKENDUTILS_API FAttributeSetConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, NoClear, Category = "Weekend Utils|Abilities|Attribute Set")
	TSubclassOf<UAttributeSet> AttributeSet = nullptr;

	UPROPERTY(EditAnywhere, NoClear, Category = "Weekend Utils|Abilities|Attribute Set")
	TObjectPtr<UDataTable> InitializationData = nullptr;
};
