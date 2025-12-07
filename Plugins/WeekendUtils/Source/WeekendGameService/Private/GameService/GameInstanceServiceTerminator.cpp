///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "GameService/GameInstanceServiceTerminator.h"

#include "WeekendGameService.h"
#include "GameService/GameServiceManager.h"

void UGameInstanceServiceTerminator::Deinitialize()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGameInstanceServiceTerminator.Deinitialize"), STAT_GameInstanceServiceTerminator_Deinitialize, STATGROUP_GameService);

	if (UGameServiceManager* ServiceManager = UGameServiceManager::FindInstance(this); IsValid(ServiceManager))
	{
		ServiceManager->Terminate();
	}

	Super::Deinitialize();
}
