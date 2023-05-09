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
#include "Subsystems/EngineSubsystem.h"

#include "GameServiceManager.generated.h"

class UGameServiceConfig;

/**
 * Mastermind behind running and maintaining game services.
 * Service classes and their desired instance classes can be registered here. Provides API to start and shutdown services,
 * while ensuring that dependencies of started services are started first, so services can always access their dependent
 * services without valid checking them.
 *
 * Although this managers lifetime exceeds the lifetime of a @UWorld, the lifetime of created services is always bound to
 * that of the world they have been started in. This requires a cooperation with @UWorldGameServiceRunner.
 *
 * See @UGameServiceConfig for how to register service classes manually and @UGameModeServiceConfigBase to automatically
 * register configured services based on the current worlds @AGameMode.
 *
 * Services are configured by UClass, which is also used for accessing the service from service users. Service classes can
 * either be derived from @UGameServiceBase or any @UInterface.
 * Configured service instance classes MUST be derived from @UGameServiceBase, but if they are registered under an interface,
 * they must also implement that interface.
 *
 * @remark It is strongly recommended to access services by deriving from @FGameServieUser and using its inherited utilities
 * instead of accessing the service manager directly.
 */
UCLASS()
class WEEKENDUTILS_API UGameServiceManager : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	/** @returns the singleton GameServiceManager, which is valid as long as the @UEngine is available. */
	FORCEINLINE static UGameServiceManager& Get() { return *GEngine->GetEngineSubsystem<UGameServiceManager>(); }

	/** @returns the singleton GameServiceManager, which is valid as long as the @UEngine is available. */
	FORCEINLINE static UGameServiceManager* GetPtr() { return GEngine ? GEngine->GetEngineSubsystem<UGameServiceManager>() : nullptr; }

	/**
	 * Registers all service classes in passed @UGameServiceConfig, as long as they have not been configured already with a higher priority.
	 * @remark Only allowed to be called before configured services have been started.
	 */
	void RegisterServices(const UGameServiceConfig& Config);

	/**
	 * Registers a service class and its instance class, as long as it has not been configured already with a higher priority.
	 * @remark Only allowed to be called before configured service has been started.
	 * @returns whether the service was registered successfully.
	 */
	bool RegisterServiceClass(const FGameServiceClass& ServiceClass, const FGameServiceInstanceClass& InstanceClass, int32 Priority = 0);

	/** Creates and starts all service instances that were previously registered, and all resulting service dependencies. */
	void StartRegisteredServices(UWorld& TargetWorld);

	/**
	 * Starts a service and its dependencies.
	 * @note If service was already started, its existing instance is returned instead.
	 * @note When starting multiple services that use the same instance class, only one instance will be created, but it
	 * will be made available through all ServiceClasses it was started for.
	 */
	UGameServiceBase& StartService(UWorld& TargetWorld, const FGameServiceClass& ServiceClass, UGameServiceBase& ServiceInstance);
	UGameServiceBase& StartService(UWorld& TargetWorld, const FGameServiceClass& ServiceClass, const FGameServiceInstanceClass& InstanceClass);
	UGameServiceBase& StartService(UWorld& TargetWorld, const FGameServiceInstanceClass& InstanceClass);

	template<typename ServiceClass>
	ServiceClass& StartService(UWorld& TargetWorld)
	{
		static_assert(!TIsAbstract<ServiceClass>::Value);
		static_assert(TIsDerivedFrom<ServiceClass, UGameServiceBase>::Value);
		return *Cast<ServiceClass>(&StartService(TargetWorld, GameService::GetServiceUClass<ServiceClass>()));
	}

	template<typename ServiceClass, class InstanceClass>
	InstanceClass& StartService(UWorld& TargetWorld)
	{
		static_assert(!TIsAbstract<InstanceClass>::Value);
		static_assert(TIsDerivedFrom<InstanceClass, ServiceClass>::Value);
		static_assert(TIsDerivedFrom<InstanceClass, UGameServiceBase>::Value);
		return *Cast<InstanceClass>(&StartService(TargetWorld, GameService::GetServiceUClass<ServiceClass>(), InstanceClass::StaticClass()));
	}

	template<typename ServiceClass>
	ServiceClass& StartService(UWorld& TargetWorld, UGameServiceBase& ServiceInstance)
	{
		return *Cast<ServiceClass>(&StartService(TargetWorld, GameService::GetServiceUClass<ServiceClass>(), ServiceInstance));
	}

	/** Attempts to start a service and its dependencies. @returns the started service, or TOptional<> if there was a problem. */
	TOptional<UGameServiceBase*> TryStartService(UWorld& TargetWorld, const FGameServiceClass& ServiceClass);

	/** @returns whether a certain service class is currently registered. */
	template<typename ServiceClass>
	bool IsServiceRegistered() const { return IsServiceRegistered(GameService::GetServiceUClass<ServiceClass>()); }
	bool IsServiceRegistered(const FGameServiceClass& ServiceClass) const;

	/** @returns whether a certain service start was already kicked-off on the service manager. */
	template<typename ServiceClass>
	bool WasServiceStarted() const { return WasServiceStarted(GameService::GetServiceUClass<ServiceClass>()); }
	bool WasServiceStarted(const FGameServiceClass& ServiceClass) const;
	bool WasServiceStarted(const UGameServiceBase* ServiceInstance) const;

	/** @returns whether a certain service start was already kicked-off on the service manager and the instance is considered 'running'. */
	template<typename ServiceClass>
	bool IsServiceRunning() const { return IsServiceRunning(GameService::GetServiceUClass<ServiceClass>()); }
	bool IsServiceRunning(const FGameServiceClass& ServiceClass) const;
	bool IsServiceRunning(const UGameServiceBase* ServiceInstance) const;

	/** @returns the started service instance for given service class, or nullptr if no instance exists. */
	UGameServiceBase* FindStartedServiceInstance(const FGameServiceClass& ServiceClass) const;

	template<typename ServiceClass, typename InstanceClass = ServiceClass>
	InstanceClass* FindStartedServiceInstance() const
	{
		static_assert(!TIsAbstract<InstanceClass>::Value);
		static_assert(TIsDerivedFrom<InstanceClass, ServiceClass>::Value);
		static_assert(TIsDerivedFrom<InstanceClass, UGameServiceBase>::Value);
		return Cast<InstanceClass>(FindStartedServiceInstance(GameService::GetServiceUClass<ServiceClass>()));
	}

	TArray<FGameServiceClass> GetAllRegisteredServiceClasses() const;
	TArray<FGameServiceInstanceClass> GetAllRegisteredServiceInstanceClasses() const;
	TOptional<FGameServiceInstanceClass> FindRegisteredServiceInstanceClass(const FGameServiceClass& ServiceClass) const;
	TArray<FGameServiceClass> GetAllStartedServiceClasses() const;
	TArray<UGameServiceBase*> GetAllStartedServiceInstances() const;

	/** @returns the service instance class that should be used for given service class, or TOptional<> if it could not be determined. */
	TOptional<FGameServiceInstanceClass> DetermineServiceInstanceClass(const FGameServiceClass& ServiceClass) const;

	/**
	 * Shuts down ALL started service instanced and removes kept lifetime references to them.
	 * Services are shut down in in the reverse order of how they were started, so they can properly deregister from their dependencies.
	 * This is called each time a world tears down by @UWorldGameServiceRunner.
	 */
	void ShutdownAllServices();

	/** Clears all service configurations that were registered. This is called each time a world tears down by @UWorldGameServiceRunner. */
	void ClearServiceRegister();

private:
	struct FServiceClassRegistryEntry
	{
		FGameServiceClass RegisterClass = nullptr;
		FGameServiceInstanceClass InstanceClass = nullptr;
		int32 RegisterPriority = 0;
	};

	/** Key: ServiceClass | Value: Registry Entry */
	TMap<FGameServiceClass, FServiceClassRegistryEntry> ServiceClassRegister;

	/**
	 * Key: ServiceClass | Value: Service Instance
	 * Values are TStrongObjectPtr<> to manually control the lifetime of created instanced.
	 */
	TMap<FGameServiceClass, TStrongObjectPtr<UGameServiceBase>> StartedServices;

	/** List of all service classes that have been started, ordered by when they were started. First started service is at [0]. */
	TArray<FGameServiceClass> StartOrderedServices;

	static UGameServiceBase* CreateServiceInstance(UWorld& OwningWorld, const FGameServiceClass& ServiceInstanceClass);
	void StartServiceDependencies(UWorld& TargetWorld, const UGameServiceBase& ServiceInstance);
};
