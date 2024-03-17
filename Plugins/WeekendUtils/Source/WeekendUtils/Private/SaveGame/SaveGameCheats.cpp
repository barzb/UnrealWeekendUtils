///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Cheat/CheatCommand.h"
#include "GameService/GameServiceLocator.h"
#include "SaveGame/SaveGameService.h"
#include "SaveGame/SaveGameEditor.h"

DEFINE_CHEAT_COLLECTION(WeekendSaveGameCheats, AsCheatMenuTab("Save Game"))
{
	DEFINE_CHEAT_COMMAND(AutosaveCheat, "Cheat.SaveGame.Autosave")
	.DisplayAs("Autosave")
	DEFINE_CHEAT_EXECUTE(AutosaveCheat)
	{
		USaveGameService* SaveGameService = UGameServiceLocator::FindService<USaveGameService>();
		if (LogInvalidity(SaveGameService, "SaveGameService not available"))
			return;

		SaveGameService->RequestAutosave();
	}

	DEFINE_CHEAT_COMMAND(LoadAutosaveCheat, "Cheat.SaveGame.LoadAutosave")
	.DisplayAs("Load Autosave")
	DEFINE_CHEAT_EXECUTE(LoadAutosaveCheat)
	{
		USaveGameService* SaveGameService = UGameServiceLocator::FindService<USaveGameService>();
		if (LogInvalidity(SaveGameService, "SaveGameService not available"))
			return;

		SaveGameService->RequestLoadCurrentSaveGameFromSlot(SaveGameService->GetAutosaveSlotName());
	}

#if WITH_EDITOR
	DEFINE_CHEAT_COMMAND(OpenSaveGameEditorCheat, "Cheat.SaveGame.OpenEditor")
	.DisplayAs("Open SaveGame Editor")
	DEFINE_CHEAT_EXECUTE(OpenSaveGameEditorCheat)
	{
		const USaveGameService* SaveGameService = UGameServiceLocator::FindService<USaveGameService>();
		if (LogInvalidity(SaveGameService, "SaveGameService not available"))
			return;

		const FCurrentSaveGame& CurrentSaveGame = SaveGameService->GetCurrentSaveGame();
		USaveGameEditor::OpenSaveGameEditor(CurrentSaveGame.GetPtr());
	}
#endif
}
