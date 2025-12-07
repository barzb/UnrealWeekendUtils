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
class WEEKENDGAMESERVICE_API UGameServiceLocator : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** @returns a service instance by ServiceClass if it was already started, or nullptr. */
	template<typename T> typename TEnableIf<TIsDerivedFrom<T, UGameServiceBase>::IsDerived, T*>::Type
	static /*(T*)*/ FindService(const UObject* WorldContext)
	{
		return Cast<T>(FindServiceInternal(WorldContext, T::StaticClass()));
	}

	/** @returns a service instance by ServiceClass if it was already started, or nullptr. */
	template<typename T> typename TEnableIf<TIsIInterface<T>::Value, T*>::Type
	static /*(T*)*/ FindService(const UObject* WorldContext)
	{
		TScriptInterface<T> ServiceInstance = FindServiceInternal(WorldContext, T::UClassType::StaticClass());
		return Cast<T>(ServiceInstance.GetInterface());
	}

	/** @returns a service instance by ServiceClass if it was already started, or nullptr. */
	template<typename T> typename TEnableIf<TIsDerivedFrom<T, UGameServiceBase>::IsDerived, TWeakObjectPtr<T>>::Type
	static /*(TWeakObjectPtr<T>)*/ FindServiceAsWeakPtr(const UObject* WorldContext)
	{
		return TWeakObjectPtr<T>(Cast<T>(FindServiceInternal(WorldContext, T::StaticClass())));
	}

	/** @returns a service instance by ServiceClass if it was already started, or nullptr. */
	template<typename T> typename TEnableIf<TIsIInterface<T>::Value, TWeakInterfacePtr<T>>::Type
	static /*(TWeakInterfacePtr<T>)*/ FindServiceAsWeakPtr(const UObject* WorldContext)
	{
		return TWeakInterfacePtr<T>(FindServiceInternal(WorldContext, T::UClassType::StaticClass()));
	}

	/** @returns a service instance by ServiceClass that is expected to be already started. */
	template<typename T> typename TEnableIf<TIsDerivedFrom<T, UGameServiceBase>::IsDerived, T&>::Type
	static /*(T&)*/ FindServiceChecked(const UObject* WorldContext)
	{
		return *Cast<T>(FindServiceInternal(WorldContext, T::StaticClass()));
	}

	/** @returns a service instance by ServiceClass that is expected to be already started. */
	template<typename T> typename TEnableIf<TIsIInterface<T>::Value, TScriptInterface<T>>::Type
	static /*(TScriptInterface<T>)*/ FindServiceChecked(const UObject* WorldContext)
	{
		UObject& ServiceInstance = *FindServiceInternal(WorldContext, T::UClassType::StaticClass());
		return TScriptInterface<T>(&ServiceInstance);
	}

	/** (Blueprint utility) @returns a service instance by ServiceClass if it was already started, or nullptr. */
	UFUNCTION(BlueprintCallable, DisplayName = "Find Service (by Interface)", Category = "Game Service",
		meta = (WorldContext = "WorldContext", DeterminesOutputType = "ServiceInterfaceClass"))
	static UObject* FindService_ByInterfaceClass(const UObject* WorldContext, TSubclassOf<UInterface> ServiceInterfaceClass);

	/** (Blueprint utility) @returns a service instance by ServiceClass if it was already started, or nullptr. */
	UFUNCTION(BlueprintCallable, DisplayName = "Find Service (by Class)", Category = "Game Service",
		meta = (WorldContext = "WorldContext", DeterminesOutputType = "ServiceClass"))
	static UObject* FindService_ByGameServiceClass(const UObject* WorldContext, TSubclassOf<UGameServiceBase> ServiceClass);

private:
	static UObject* FindServiceInternal(const UObject* WorldContext, const TSubclassOf<UObject>& ServiceClass);
};
