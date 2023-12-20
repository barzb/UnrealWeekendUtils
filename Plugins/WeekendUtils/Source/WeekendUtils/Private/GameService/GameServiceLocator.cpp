///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "GameService/GameServiceLocator.h"

#include "GameService/GameServiceBase.h"
#include "GameService/GameServiceManager.h"

UObject* UGameServiceLocator::FindService_ByInterfaceClass(TSubclassOf<UInterface> ServiceInterfaceClass)
{
	return FindServiceInternal(*ServiceInterfaceClass);
}

UObject* UGameServiceLocator::FindService_ByGameServiceClass(TSubclassOf<UGameServiceBase> ServiceClass)
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
