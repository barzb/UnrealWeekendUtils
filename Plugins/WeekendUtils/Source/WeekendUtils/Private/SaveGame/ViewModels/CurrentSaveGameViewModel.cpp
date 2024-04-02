///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "SaveGame/ViewModels/CurrentSaveGameViewModel.h"

#include "SaveGame/SaveGameService.h"

UCurrentSaveGameViewModel::UCurrentSaveGameViewModel()
{
	ServiceDependencies.Add<USaveGameService>();
}

void UCurrentSaveGameViewModel::BeginUsage()
{
	USaveGameService& SaveGameService = UseGameService<USaveGameService>(this);
	SaveGameService.OnAfterRestored.AddUObject(this, &ThisClass::UpdateForCurrentSaveGame);

	UpdateForCurrentSaveGame(SaveGameService.GetCurrentSaveGame());
}

void UCurrentSaveGameViewModel::ContinueSaveGame()
{
	if (bCanContinue)
	{
		UseGameService<USaveGameService>(this).TryTravelIntoCurrentSaveGame();
	}
}

void UCurrentSaveGameViewModel::CreateNewGame()
{
	UseGameService<USaveGameService>(this).CreateAndRestoreNewSaveGameAsCurrent();
}

void UCurrentSaveGameViewModel::BeginDestroy()
{
	EndUsage();

	Super::BeginDestroy();
}

void UCurrentSaveGameViewModel::UpdateForCurrentSaveGame(const FCurrentSaveGame& CurrentSaveGame)
{
	UE_MVVM_SET_PROPERTY_VALUE(bCanContinue, (CurrentSaveGame.IsValid() && !CurrentSaveGame.IsNewGame()));
}

void UCurrentSaveGameViewModel::EndUsage()
{
	if (USaveGameService* SaveGameService = FindOptionalGameService<USaveGameService>().Get())
	{
		SaveGameService->OnAfterRestored.RemoveAll(this);
	}
}
