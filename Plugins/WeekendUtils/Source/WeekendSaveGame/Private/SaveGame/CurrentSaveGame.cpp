///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "SaveGame/CurrentSaveGame.h"

FCurrentSaveGame FCurrentSaveGame::CreateFromNewGame(USaveGame& SaveGame)
{
	FCurrentSaveGame CurrentSaveGame;
	CurrentSaveGame.SaveGameObject = &SaveGame;
	CurrentSaveGame.bWasEverLoaded = false;
	return CurrentSaveGame;
}

FCurrentSaveGame FCurrentSaveGame::CreateFromLoadedGame(USaveGame& SaveGame, TOptional<FSlotName> LoadedFromSlotName)
{
	FCurrentSaveGame CurrentSaveGame;
	CurrentSaveGame.SaveGameObject = &SaveGame;
	CurrentSaveGame.bWasEverLoaded = true;
	CurrentSaveGame.SlotLastRestoredFrom = LoadedFromSlotName;
	CurrentSaveGame.SlotLastSavedTo = LoadedFromSlotName;
	return CurrentSaveGame;
}

bool FCurrentSaveGame::operator==(const FCurrentSaveGame& Other) const
{
	return (SaveGameObject == Other.SaveGameObject);
}

bool FCurrentSaveGame::operator!=(const FCurrentSaveGame& Other) const
{
	return (SaveGameObject != Other.SaveGameObject);
}

bool FCurrentSaveGame::operator==(const USaveGame& OtherSaveGame) const
{
	return (SaveGameObject == &OtherSaveGame);
}

bool FCurrentSaveGame::operator!=(const USaveGame& OtherSaveGame) const
{
	return (SaveGameObject != &OtherSaveGame);
}

void FCurrentSaveGame::Reset()
{
	SaveGameObject = nullptr;
	SlotLastSavedTo.Reset();
	SlotLastRestoredFrom.Reset();
	UtcTimeOfLastSave.Reset();
	bWasEverLoaded = false;
}
