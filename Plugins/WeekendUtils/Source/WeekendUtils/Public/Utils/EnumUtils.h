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
#include "Misc/Optional.h"

/** Defines LexFromString and LexToString as well as discrete enum conversion functions for given enum type. */
#define DEFINE_ENUM_STRING_CONVERTERS(EnumName, EnumType) \
	inline FString LexToString(const EnumType& Type) { return WeekendUtils::EnumToString<EnumType>(Type); } \
	inline bool LexFromString(EnumType& OutType, const TCHAR* StringBuffer) { return WeekendUtils::EnumFromString<EnumType>(OUT OutType, StringBuffer); } \
	inline FString EnumName##ToString(const EnumType& Type) { return WeekendUtils::EnumToString<EnumType>(Type); } \
	inline bool EnumName##FromString(EnumType& OutType, const TCHAR* StringBuffer) { return WeekendUtils::EnumFromString<EnumType>(OUT OutType, StringBuffer); } \
	inline TOptional<EnumType> EnumName##FromString(const TCHAR* StringBuffer) { return WeekendUtils::EnumFromString<EnumType>(StringBuffer); } 

/** Defines discrete enum conversion functions like EnumFromBool and EnumToBool for given enum type. */
#define DEFINE_ENUM_BOOL_CONVERTERS(EnumName, EnumType, TrueValue, FalseValue) \
	inline EnumType EnumName##FromBool(const bool bIs##TrueValue) { return bIs##TrueValue ? EnumType::TrueValue : EnumType::FalseValue; } \
	inline bool EnumName##ToBool(const EnumType& Value) { return Value == EnumType::TrueValue; }

///////////////////////////////////////////////////////////////////////////////////////

namespace WeekendUtils
{
	/** Converts given enum class value into a string. */
	template <typename EnumType, typename = typename TEnableIf<TIsEnumClass<EnumType>::Value>::Type>
	FString EnumToString(const EnumType& Value)
	{
		const UEnum* Enum = StaticEnum<EnumType>();
		return Enum->GetNameStringByValue(static_cast<int64>(Value));
	}

	/** Attempts to parse a string into given enum class value out param. @returns whether the operation was successful. */
	template <typename EnumType, typename = typename TEnableIf<TIsEnumClass<EnumType>::Value>::Type>
	bool EnumFromString(EnumType& OuEnumTypeValue, const TCHAR* StringBuffer)
	{
		const UEnum* Enum = StaticEnum<EnumType>();
		const int64 FoundValue = Enum->GetValueByNameString(StringBuffer);
		if (FoundValue == INDEX_NONE)
			return false;

		OuEnumTypeValue = static_cast<EnumType>(FoundValue);
		return true;
	}
	/** Attempts to parse a string into given enum class value out param. @returns whether the operation was successful. */
	template <typename EnumType, typename = typename TEnableIf<TIsEnumClass<EnumType>::Value>::Type>
	TOptional<EnumType> EnumFromString(const TCHAR* StringBuffer)
	{
		EnumType Value;
		return EnumFromString(OUT Value, StringBuffer) ? Value : TOptional<EnumType>();
	}
}
