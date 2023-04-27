///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz in cooperation with Nine Worlds Studios GmbH.
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See accompanying file LICENSE.
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
	struct TDependencyList
	{
		template<typename T> void Add() { Add(GetDependencyUClass<T>()); }
		void Add(const TSubclassOf<ElementType>& Element) { Elements.AddUnique(Element); }

		template<typename T> bool Contains() const { return Contains(GetDependencyUClass<T>()); }
		bool Contains(const TSubclassOf<ElementType>& Element) const { return Elements.Contains(Element); }

		bool IsEmpty() const { return Elements.IsEmpty(); }
		int32 Num() const { return Elements.Num(); }

		FORCEINLINE const TArray<TSubclassOf<ElementType>>& ToArray() const { return Elements; }
		TArray<TSubclassOf<ElementType>> Elements;
	};
}

using FGameServiceDependencies = GameService::TDependencyList<UObject>;
using FSubsystemDependencies = GameService::TDependencyList<USubsystem>;
