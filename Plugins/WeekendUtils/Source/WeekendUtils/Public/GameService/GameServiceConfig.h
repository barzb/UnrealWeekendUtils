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
	 * @note Be aware that some service users (like WorldSubsystems) could already have started services.
	 * @param World The world used as outer for the new config instance.
	 * @param ConfigExec External function to be called to configure the config instance before it is registered.
	 * @returns the created and already registered service config.
	 *
	 * @example:
	 * UGameServiceConfig::CreateForWorld(World, [](UGameServiceConfig& Config)
	 * {
	 *    Config.SetPriority(7);
	 *    Config.AddSingletonService<USomeService>();
	 *    Config.AddSingletonService<IAnotherServiceInterface, UAnotherServiceImpl>();
	 * });
	 */
	static UGameServiceConfig& CreateForWorld(UWorld& World, TFunction<void(UGameServiceConfig&)> ConfigExec);

	/**
	 * Creates a UGameServiceConfig instance for the next world that will start, automatically registering it with the @UGameServiceManager.
	 * @note This is mainly intended to be used in automation tests, before a test world is created.
	 * @param ConfigExec External function to be called to configure the config instance before it is registered.
	 * @returns the created and already registered service config.
	 *
	 * @example:
	 * UGameServiceConfig::CreateForNextWorld([](UGameServiceConfig& Config)
	 * {
	 *    Config.SetPriority(7);
	 *    Config.AddSingletonService<USomeService>();
	 *    Config.AddSingletonService<IAnotherServiceInterface, UAnotherServiceImpl>();
	 * });
	 */
	static UGameServiceConfig& CreateForNextWorld(TFunction<void(UGameServiceConfig&)> ConfigExec);

	/** Automatically registers the config instance with the @UGameServiceManager. Already called when using @CreateForWorld(). */
	void RegisterWithGameServiceManager() const;

	/**
	 * Configures a game service class to be registered.
	 * @note Singleton services are only instanced once (per world) for the register-type.
	 * @note Services that are registered with the same InstanceClass will share the same instance.
	 */
	template<typename ServiceClass, class InstanceClass = ServiceClass>
	void AddSingletonService()
	{
		static_assert(!TIsAbstract<InstanceClass>::Value);
		static_assert(TIsDerivedFrom<InstanceClass, ServiceClass>::Value);
		static_assert(TIsDerivedFrom<InstanceClass, UGameServiceBase>::Value);
		SingletonServices.Add(GameService::GetServiceUClass<ServiceClass>(), InstanceClass::StaticClass());

		//#todo-service-later return some sort of struct ref to the service config entry where additional dependencies can be configured
	}

	FORCEINLINE int32 GetNumConfiguredServices() const { return SingletonServices.Num(); }
	FORCEINLINE const TMap<FGameServiceClass, FGameServiceInstanceClass>& GetConfiguredServices() const
	{
		return SingletonServices; //#todo-service-later consider supporting ScopedServices as well
	}

	/** Configs with a higher priority will overwrite service registrations from configs with lower priority. */
	FORCEINLINE void SetPriority(uint32 NewPriority) { RegisterPriority = NewPriority; }
	FORCEINLINE uint32 GetPriority() const { return RegisterPriority; }

protected:
	UPROPERTY()
	uint32 RegisterPriority = 0;

	/** Key: FGameServiceClass | Value: FGameServiceInstanceClass */
	UPROPERTY()
	TMap<TSubclassOf<UObject>, TSubclassOf<UGameServiceBase>> SingletonServices;
};
