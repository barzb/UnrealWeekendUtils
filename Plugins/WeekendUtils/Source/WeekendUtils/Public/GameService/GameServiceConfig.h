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
#include "GameService/GameServiceBase.h"
#include "GameService/GameServiceUtils.h"
#include "UObject/Object.h"

#include "GameServiceConfig.generated.h"

/**
 * Configuration container for the @UGameServiceManager.
 */
UCLASS(CollapseCategories, NotBlueprintable, NotBlueprintType)
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
	 *    Config.AddService<USomeService>();
	 *    Config.AddService<IAnotherServiceInterface, UAnotherServiceImpl>();
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
	 *    Config.AddService<USomeService>();
	 *    Config.AddService<IAnotherServiceInterface, UAnotherServiceImpl>();
	 * });
	 */
	static UGameServiceConfig& CreateForNextWorld(TFunction<void(UGameServiceConfig&)> ConfigExec);

	/** Automatically registers the config instance with the @UGameServiceManager. Already called when using @CreateForWorld(). */
	void RegisterWithGameServiceManager() const;

	/**
	 * Configures a game service class to be registered.
	 * @note Services are only instanced once (=> singleton) for the register-type.
	 * @note Services that are registered with the same InstanceClass will share the same instance.
	 */
	template<class ServiceClass, class InstanceClass = ServiceClass>
	void AddService()
	{
		static_assert(!TIsAbstract<InstanceClass>::Value);
		static_assert(TIsDerivedFrom<InstanceClass, ServiceClass>::Value);
		static_assert(TIsDerivedFrom<InstanceClass, UGameServiceBase>::Value);
		ConfiguredServices.Add(GameService::GetServiceUClass<ServiceClass>(), InstanceClass::StaticClass());
	}

	/**
	 * Configures a game service class to be registered.
	 * @note Services are only instanced once (=> singleton) for the register-type.
	 * @note Services that are registered with the same InstanceClass will share the same instance.
	 */
	template<class ServiceClass>
	void AddService(const FGameServiceInstanceClass& InstanceClass)
	{
		check(InstanceClass != nullptr);
		check(InstanceClass->IsChildOf<ServiceClass>());
		check(!InstanceClass->HasAnyClassFlags(CLASS_Abstract));
		ConfiguredServices.Add(GameService::GetServiceUClass<ServiceClass>(), InstanceClass);
	}

	/**
	 * Configures a game service class to be registered.
	 * The created service instance will be based on provided template object (can be CDO). 
	 * @note Services are only instanced once (=> singleton) for the register-type.
	 * @note Services that are registered with the same InstanceClass will share the same instance.
	 */
	template<class ServiceClass>
	void AddService(const UGameServiceBase& TemplateInstance)
	{
		check(!TemplateInstance.GetClass()->HasAnyClassFlags(CLASS_Abstract));
		const TSubclassOf<UObject> RegisterClass = GameService::GetServiceUClass<ServiceClass>();
		ConfiguredServices.Add(RegisterClass, TemplateInstance.GetClass());
		ConfiguredTemplates.Add(RegisterClass, &TemplateInstance);
	}

	FORCEINLINE int32 GetNumConfiguredServices() const { return ConfiguredServices.Num(); }
	FORCEINLINE const TMap<FGameServiceClass, FGameServiceInstanceClass>& GetConfiguredServices() const
	{
		return ConfiguredServices;
	}
	FORCEINLINE const UGameServiceBase* GetConfiguredServiceTemplate(const FGameServiceClass& RegisterClass) const
	{
		return (ConfiguredTemplates.Contains(RegisterClass) ? ConfiguredTemplates[RegisterClass] : nullptr);
	}

	/** Configs with a higher priority will overwrite service registrations from configs with lower priority. */
	FORCEINLINE void SetPriority(uint32 NewPriority) { ConfiguredPriority = NewPriority; }
	FORCEINLINE uint32 GetPriority() const { return ConfiguredPriority; }

protected:
	/** Key: FGameServiceClass | Value: FGameServiceInstanceClass */
	UPROPERTY(VisibleAnywhere)
	TMap<TSubclassOf<UObject>, TSubclassOf<UGameServiceBase>> ConfiguredServices;

	/** Key: FGameServiceClass | Value: UGameService Instance */
	UPROPERTY(VisibleAnywhere)
	TMap<TSubclassOf<UObject>, TObjectPtr<const UGameServiceBase>> ConfiguredTemplates;

	UPROPERTY(VisibleAnywhere)
	uint32 ConfiguredPriority = 0;

	void ResetConfiguredServices();
};
