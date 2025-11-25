///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "SaveGame/ViewModels/SaveGameMenuViewModel.h"

#include "SaveGame/SaveGameService.h"

bool USaveGameMenuViewModel::ShouldShowLoadButton() const
{
	return SaveGameService && SaveGameService->IsLoadingAllowed() && SaveGameService->HasAnyCachedSaveGameSnapshot();
}

bool USaveGameMenuViewModel::ShouldShowSaveButton() const
{
	return SaveGameService && SaveGameService->IsSavingAllowed() && SaveGameService->GetCurrentSaveGame().IsValid();
}

void USaveGameMenuViewModel::BeginUsage()
{
	Super::BeginUsage();

	SaveGameService->OnAvailableSaveGamesChanged.AddUObject(this, &ThisClass::UpdateSaveLoadButtonAvailability);
	UpdateSaveLoadButtonAvailability();
}

void USaveGameMenuViewModel::EndUsage()
{
	if (SaveGameService)
	{
		SaveGameService->OnAvailableSaveGamesChanged.RemoveAll(this);
	}

	Super::EndUsage();
}

void USaveGameMenuViewModel::UpdateForCurrentSaveGame(const FCurrentSaveGame& CurrentSaveGame)
{
	Super::UpdateForCurrentSaveGame(CurrentSaveGame);

	UpdateSaveLoadButtonAvailability();
}

void USaveGameMenuViewModel::UpdateSaveLoadButtonAvailability()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ShouldShowLoadButton);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ShouldShowSaveButton);
}
