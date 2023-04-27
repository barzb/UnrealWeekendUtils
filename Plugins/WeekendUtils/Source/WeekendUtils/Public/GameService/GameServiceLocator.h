// (c) 2023 Nine Worlds Studios GmbH. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameService/GameServiceUser.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "GameServiceLocator.generated.h"

/**
 * Static function library for conveniently locating game services that might exist in the world.
 * (!) Native code classes should consider deriving from @FGameServiceUser instead of using this locator.
 * #todo-service add docs
 */
UCLASS()
class WEEKENDUTILS_API UGameServiceLocator : public UBlueprintFunctionLibrary, public FGameServiceUser
{
	GENERATED_BODY()

public:
	template<typename T> typename TEnableIf<TIsDerivedFrom<T, UGameServiceBase>::IsDerived, TWeakObjectPtr<T>>::Type
	static /*(TWeakObjectPtr<T>)*/ FindService()
	{
		return TWeakObjectPtr<T>(Cast<T>(FindServiceInternal(T::StaticClass())));
	}

	template<typename T> typename TEnableIf<TIsIInterface<T>::Value, TWeakInterfacePtr<T>>::Type
	static /*(TWeakInterfacePtr<T>)*/ FindService()
	{
		return TWeakInterfacePtr<T>(FindServiceInternal(T::UClassType::StaticClass()));
	}

	UFUNCTION(BlueprintCallable, DisplayName = "FindService", Category = "Game Service", meta = (DeterminesOutputType = "ServiceInterfaceClass"))
	static UObject* FindService_ByInterfaceClass(const TSubclassOf<UInterface>& ServiceInterfaceClass);

	UFUNCTION(BlueprintCallable, DisplayName = "FindService", Category = "Game Service", meta = (DeterminesOutputType = "ServiceClass"))
	static UObject* FindService_ByGameServiceClass(const TSubclassOf<UGameServiceBase>& ServiceClass);

private:
	static UObject* FindServiceInternal(const TSubclassOf<UObject>& ServiceClass);
};