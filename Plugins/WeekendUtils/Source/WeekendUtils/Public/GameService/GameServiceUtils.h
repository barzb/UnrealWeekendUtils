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

class UGameServiceBase;
class USubsystem;

namespace GameService
{
	/** @returns the UClass of the typed service. */
	template<typename T> // IInterface version
	static typename TEnableIf<TIsIInterface<T>::Value, UClass*>::Type GetServiceUClass()
	{
		return T::UClassType::StaticClass();
	}

	/** @returns the UClass of the typed service. */
	template<typename T> // UGameServiceBase version
	static typename TEnableIf<TIsDerivedFrom<T, UGameServiceBase>::IsDerived, UClass*>::Type GetServiceUClass()
	{
		return T::StaticClass();
	}

	/** @returns the UClass of the typed service dependency. */
	template<typename T> // IInterface version
	static typename TEnableIf<TIsIInterface<T>::Value, UClass*>::Type GetDependencyUClass()
	{
		return T::UClassType::StaticClass();
	}

	/** @returns the UClass of the typed service dependency. */
	template<typename T> // UObject version
	static typename TEnableIf<TIsDerivedFrom<T, UObject>::IsDerived, UClass*>::Type GetDependencyUClass()
	{
		return T::StaticClass();
	}

	/**
	 * Utility container nesting a TArray<TSubclassOf<ElementType>> list of dependency classes.
	 * @tparam ElementType Base class type of the dependencies that can be stored in this list.
	 */
	template<typename ElementType>
	struct TDependencyList : TArray<TSubclassOf<ElementType>>
	{
		template<typename T> void Add() { Add(GetDependencyUClass<T>()); }
		template<typename T> bool Contains() const { return Contains(GetDependencyUClass<T>()); }

		using TArray<TSubclassOf<ElementType>>::Add;
		using TArray<TSubclassOf<ElementType>>::Contains;
		using TArray<TSubclassOf<ElementType>>::operator=;
	};
}

using FGameServiceDependencies = GameService::TDependencyList<UObject>;
using FSubsystemDependencies = GameService::TDependencyList<USubsystem>;
