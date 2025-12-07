///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "GameService/GameServiceConfig.h"

#include "WeekendGameService.h"
#include "GameService/GameServiceManager.h"
#include "GameService/WorldGameServiceRunner.h"

UGameServiceConfig& UGameServiceConfig::CreateForWorld(const UWorld& World, TFunction<void(UGameServiceConfig&)> ConfigExec)
{
	UGameServiceConfig* NewConfig = NewObject<UGameServiceConfig>(World.GetGameInstance());
	ConfigExec(*NewConfig);
	NewConfig->RegisterWithGameServiceManager(World);
	return *NewConfig;
}

UGameServiceConfig& UGameServiceConfig::CreateForNextWorld(TFunction<void(UGameServiceConfig&)> ConfigExec)
{
	UGameServiceConfig* NewConfig = NewObject<UGameServiceConfig>(GetTransientPackage());
	ConfigExec(*NewConfig);
	UWorldGameServiceRunner::SetServiceConfigForNextWorld(*NewConfig);
	return *NewConfig;
}

void UGameServiceConfig::RegisterWithGameServiceManager(const UWorld& World) const
{
	UE_LOG(LogGameService, Verbose, TEXT("Registering GameServiceConfig (%s) with GameServiceManager"), *GetName());
	UGameServiceManager::SummonInstance(&World).RegisterServices(*this);
}

void UGameServiceConfig::ValidateDependenciesForConfiguredServices() const
{
	for (const TTuple<TSubclassOf<UObject>, TSubclassOf<UGameServiceBase>>& ServiceItr : ConfiguredServices)
	{
		const UGameServiceBase* ServiceInstance = ConfiguredTemplates.Contains(ServiceItr.Key)
			? ConfiguredTemplates[ServiceItr.Key].Get()
			: GetDefault<UGameServiceBase>(ServiceItr.Value);
		CheckServiceDependencies(*ServiceInstance);
	}
}

void UGameServiceConfig::ResetConfiguredServices()
{
	ConfiguredServices.Reset();
	ConfiguredTemplates.Reset();
}

void UGameServiceConfig::CheckServiceDependencies(const UGameServiceBase& ServiceInstance) const
{
	for (TSubclassOf<UObject> ServiceClassDependency : ServiceInstance.GetServiceClassDependencies())
	{
		checkf(ConfiguredServices.Contains(ServiceClassDependency), TEXT("%s configured a GameServiceDependency to %s, which was not configured in %s"),
			*ServiceInstance.GetClass()->GetName(), *GetNameSafe(ServiceClassDependency), *GetName());
	}
}
