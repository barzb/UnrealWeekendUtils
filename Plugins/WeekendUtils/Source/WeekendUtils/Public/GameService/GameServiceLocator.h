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
#include "GameService/GameServiceUser.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "GameServiceLocator.generated.h"

/**
 * Static function library for conveniently locating game services that might exist in the world.
 * (!) Native code classes should consider deriving from @FGameServiceUser instead of using this locator.
 */
UCLASS()
class WEEKENDUTILS_API UGameServiceLocator : public UBlueprintFunctionLibrary, public FGameServiceUser
{
	GENERATED_BODY()

public:
	/** @returns a service instance by ServiceClass if it was already started, or nullptr. */
	template<typename T> typename TEnableIf<TIsDerivedFrom<T, UGameServiceBase>::IsDerived, T*>::Type
	static /*(T*)*/ FindService()
	{
		return Cast<T>(FindServiceInternal(T::StaticClass()));
	}

	/** @returns a service instance by ServiceClass if it was already started, or nullptr. */
	template<typename T> typename TEnableIf<TIsIInterface<T>::Value, T*>::Type
	static /*(T*)*/ FindService()
	{
		TScriptInterface<T> ServiceInstance = FindServiceInternal(T::UClassType::StaticClass());
		return Cast<T>(ServiceInstance.GetInterface());
	}

	/** @returns a service instance by ServiceClass if it was already started, or nullptr. */
	template<typename T> typename TEnableIf<TIsDerivedFrom<T, UGameServiceBase>::IsDerived, TWeakObjectPtr<T>>::Type
	static /*(TWeakObjectPtr<T>)*/ FindServiceAsWeakPtr()
	{
		return TWeakObjectPtr<T>(Cast<T>(FindServiceInternal(T::StaticClass())));
	}

	/** @returns a service instance by ServiceClass if it was already started, or nullptr. */
	template<typename T> typename TEnableIf<TIsIInterface<T>::Value, TWeakInterfacePtr<T>>::Type
	static /*(TWeakInterfacePtr<T>)*/ FindServiceAsWeakPtr()
	{
		return TWeakInterfacePtr<T>(FindServiceInternal(T::UClassType::StaticClass()));
	}

	/** @returns a service instance by ServiceClass that is expected to be already started. */
	template<typename T> typename TEnableIf<TIsDerivedFrom<T, UGameServiceBase>::IsDerived, T&>::Type
	static /*(T&)*/ FindServiceChecked()
	{
		return *Cast<T>(FindServiceInternal(T::StaticClass()));
	}

	/** @returns a service instance by ServiceClass that is expected to be already started. */
	template<typename T> typename TEnableIf<TIsIInterface<T>::Value, TScriptInterface<T>>::Type
	static /*(TScriptInterface<T>)*/ FindServiceChecked()
	{
		UObject& ServiceInstance = *FindServiceInternal(T::UClassType::StaticClass());
		return TScriptInterface<T>(&ServiceInstance);
	}

	/** (Blueprint utility) @returns a service instance by ServiceClass if it was already started, or nullptr. */
	UFUNCTION(BlueprintCallable, DisplayName = "Find Service (by Interface)", Category = "Game Service", meta = (DeterminesOutputType = "ServiceInterfaceClass"))
	static UObject* FindService_ByInterfaceClass(TSubclassOf<UInterface> ServiceInterfaceClass);

	/** (Blueprint utility) @returns a service instance by ServiceClass if it was already started, or nullptr. */
	UFUNCTION(BlueprintCallable, DisplayName = "Find Service (by Class)", Category = "Game Service", meta = (DeterminesOutputType = "ServiceClass"))
	static UObject* FindService_ByGameServiceClass(TSubclassOf<UGameServiceBase> ServiceClass);

private:
	static UObject* FindServiceInternal(const TSubclassOf<UObject>& ServiceClass);
};