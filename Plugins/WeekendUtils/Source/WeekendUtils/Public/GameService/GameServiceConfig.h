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
#include "GameService/GameServiceBase.h"
#include "GameService/GameServiceUtils.h"
#include "UObject/Object.h"

#include "GameServiceConfig.generated.h"

#define GAMESERVICE_PRIO_MAX UINT32_MAX
#define GAMESERVICE_PRIO_DEFAULT 0

/**
 * Configuration container for the @UGameServiceManager.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class WEEKENDUTILS_API UGameServiceConfig : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Creates a UGameServiceConfig instance for given world, automatically registering it with the @UGameServiceManager.
	 * @param World The world used as outer for the new config instance.
	 * @param Priority Services registered by this container will overwrite existing service configs when priority is higher.
	 * @param ConfigExec External function to be called to configure the config instance before it is registered.
	 * @returns the created and already registered service config.
	 *
	 * @example:
	 * UGameServiceConfig::CreateForWorld(World, UINT32_MAX, [](UGameServiceConfig& Config)
	 * {
	 *    Config.AddSingletonService<USomeService>();
	 *    Config.AddSingletonService<IAnotherServiceInterface, UAnotherServiceImpl>();
	 * });
	 */
	static UGameServiceConfig& CreateForWorld(UWorld& World, uint32 Priority, TFunction<void(UGameServiceConfig&)> ConfigExec);

	/** Automatically registers the config instance with the @UGameServiceManager. Already called when using @CreateForWorld(). */
	void RegisterWithGameServiceManager() const;

	/**
	 * Configures a game service class to be registered. Template argument is used as register-type and instance-type.
	 * @note Singleton services are only instanced once (per world) for the register-type.
	 */
	template<typename T>
	void AddSingletonService()
	{
		static_assert(!TIsAbstract<T>::Value);
		static_assert(TIsDerivedFrom<T, UGameServiceBase>::Value);
		SingletonServices.Add(T::StaticClass(), T::StaticClass());
	}

	/**
	 * Configures a game service class to be registered.
	 * @note Singleton services are only instanced once (per world) for the register-type.
	 * @note Services that are registered with the same InstanceClass will share the same instance.
	 */
	template<typename ServiceClass, class InstanceClass>
	void AddSingletonService()
	{
		static_assert(!TIsAbstract<InstanceClass>::Value);
		static_assert(TIsDerivedFrom<InstanceClass, ServiceClass>::Value);
		static_assert(TIsDerivedFrom<InstanceClass, UGameServiceBase>::Value);
		SingletonServices.Add(GameService::GetServiceUClass<ServiceClass>(), InstanceClass::StaticClass());
	}

	FORCEINLINE const TMap<FGameServiceClass, FGameServiceInstanceClass>& GetConfiguredServices() const
	{
		return SingletonServices; //#todo-service-later consider supporting ScopedServices as well
	}

	/** Configs with a higher priority will overwrite service registrations from configs with lower priority. */
	FORCEINLINE void SetPriority(uint32 NewPriority) { RegisterPriority = NewPriority; }
	FORCEINLINE uint32 GetPriority() const { return RegisterPriority; }

protected:
	UPROPERTY()
	uint32 RegisterPriority = GAMESERVICE_PRIO_DEFAULT;

	/** Key: FGameServiceClass | Value: FGameServiceInstanceClass */
	UPROPERTY()
	TMap<TSubclassOf<UObject>, TSubclassOf<UGameServiceBase>> SingletonServices;
};
