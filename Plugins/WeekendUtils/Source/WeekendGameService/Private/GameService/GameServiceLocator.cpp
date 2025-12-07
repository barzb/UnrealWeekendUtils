///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "GameService/GameServiceLocator.h"

#include "GameService/GameServiceBase.h"
#include "GameService/GameServiceManager.h"

UObject* UGameServiceLocator::FindService_ByInterfaceClass(const UObject* WorldContext, TSubclassOf<UInterface> ServiceInterfaceClass)
{
	return FindServiceInternal(WorldContext, *ServiceInterfaceClass);
}

UObject* UGameServiceLocator::FindService_ByGameServiceClass(const UObject* WorldContext, TSubclassOf<UGameServiceBase> ServiceClass)
{
	return FindServiceInternal(WorldContext, *ServiceClass);
}

UObject* UGameServiceLocator::FindServiceInternal(const UObject* WorldContext, const TSubclassOf<UObject>& ServiceClass)
{
	const UGameServiceManager* GameServiceManager = UGameServiceManager::FindInstance(WorldContext);
	if (!IsValid(GameServiceManager))
		return nullptr;

	UObject* ServiceInstance = GameServiceManager->FindStartedServiceInstance(ServiceClass);
	return (IsValid(ServiceInstance) ? ServiceInstance : nullptr);
}
