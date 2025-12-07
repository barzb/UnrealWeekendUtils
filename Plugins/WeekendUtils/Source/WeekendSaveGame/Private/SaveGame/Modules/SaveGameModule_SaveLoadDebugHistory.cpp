///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "SaveGame/Modules/SaveGameModule_SaveLoadDebugHistory.h"

#include "GameService/GameServiceLocator.h"
#include "SaveGame/SaveGameService.h"

void USaveGameModule_SaveLoadDebugHistory::PreSaveModule()
{
	Super::PreSaveModule();

	if (!SaveGameService)
	{
		SaveGameService = UGameServiceLocator::FindService<USaveGameService>(this);
	}
	if (SaveGameService)
	{
		DebugHistory = SaveGameService->GetDebugHistory();
	}
}
