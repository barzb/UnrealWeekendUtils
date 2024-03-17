///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "SaveGame/ViewModels/CurrentSaveGame_VM.h"

#include "SaveGame/SaveGameService.h"

UCurrentSaveGame_VM::UCurrentSaveGame_VM()
{
	ServiceDependencies.Add<USaveGameService>();
}

void UCurrentSaveGame_VM::BeginUsage()
{
	USaveGameService& SaveGameService = UseGameService<USaveGameService>(this);
	SaveGameService.OnAfterRestored.AddUObject(this, &ThisClass::UpdateForCurrentSaveGame);

	UpdateForCurrentSaveGame(SaveGameService.GetCurrentSaveGame());
}

void UCurrentSaveGame_VM::ContinueSaveGame()
{
	if (bCanContinue)
	{
		UseGameService<USaveGameService>(this).TryTravelIntoCurrentSaveGame();
	}
}

void UCurrentSaveGame_VM::CreateNewGame()
{
	UseGameService<USaveGameService>(this).CreateAndRestoreNewSaveGameAsCurrent();
}

void UCurrentSaveGame_VM::BeginDestroy()
{
	EndUsage();

	Super::BeginDestroy();
}

void UCurrentSaveGame_VM::UpdateForCurrentSaveGame(const FCurrentSaveGame& CurrentSaveGame)
{
	UE_MVVM_SET_PROPERTY_VALUE(bCanContinue, (CurrentSaveGame.IsValid() && !CurrentSaveGame.IsNewGame()));
}

void UCurrentSaveGame_VM::EndUsage()
{
	if (USaveGameService* SaveGameService = FindOptionalGameService<USaveGameService>().Get())
	{
		SaveGameService->OnAfterRestored.RemoveAll(this);
	}
}
