// (c) 2023 Nine Worlds Studios GmbH. All rights reserved.

#include "GameService/GameServiceLocator.h"

#include "GameService/GameServiceBase.h"
#include "GameService/GameServiceManager.h"

UObject* UGameServiceLocator::FindService_ByInterfaceClass(const TSubclassOf<UInterface>& ServiceInterfaceClass)
{
	return FindServiceInternal(*ServiceInterfaceClass);
}

UObject* UGameServiceLocator::FindService_ByGameServiceClass(const TSubclassOf<UGameServiceBase>& ServiceClass)
{
	return FindServiceInternal(*ServiceClass);
}

UObject* UGameServiceLocator::FindServiceInternal(const TSubclassOf<UObject>& ServiceClass)
{
	const UGameServiceManager* GameServiceManager = UGameServiceManager::GetPtr();
	if (!IsValid(GameServiceManager))
		return nullptr;

	return GameServiceManager->FindStartedServiceInstance(ServiceClass);
}
