///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "GameInstanceServiceDestroyer.h"

#include "GameService/GameServiceBase.h"
#include "GameService/GameServiceManager.h"
#include "WeekendUtils.h"

void UGameInstanceServiceDestroyer::Deinitialize()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGameInstanceServiceDestroyer.Deinitialize"), STAT_GameInstanceServiceDestroyer_Deinitialize, STATGROUP_GameService);

	if (UGameServiceManager* ServiceManager = UGameServiceManager::GetPtr(); IsValid(ServiceManager))
	{
		ServiceManager->ShutdownAllServices();
		ServiceManager->ClearServiceRegister(EGameServiceLifetime::ShutdownWithGameInstance);
	}

	Super::Deinitialize();
}
