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

FGameServiceUserConfig UCurrentSaveGameViewModel::ConfigureGameServiceUser() const
{
	return FGameServiceUserConfig(this)
		.AddServiceDependency<USaveGameService>();
}

void UCurrentSaveGameViewModel::BeginUsage()
{
	SaveGameService = UseGameServiceAsPtr<USaveGameService>();
	SaveGameService->OnAfterRestored.AddUObject(this, &ThisClass::UpdateForCurrentSaveGame);
	SaveGameService->OnAvailableSaveGamesChanged.AddUObject(this, &ThisClass::UpdateTimeSinceLastSave);

	UpdateForCurrentSaveGame(SaveGameService->GetCurrentSaveGame());
}

void UCurrentSaveGameViewModel::ContinueSaveGame()
{
	if (bCanContinue)
	{
		UseGameService<USaveGameService>().TryTravelIntoCurrentSaveGame();
	}
}

void UCurrentSaveGameViewModel::CreateNewGame()
{
	UseGameService<USaveGameService>().CreateAndRestoreNewSaveGameAsCurrent();
}

FTimespan UCurrentSaveGameViewModel::GetTimespanSinceLastSave() const
{
	return FDateTime::UtcNow() - UtcTimeOfLastSave;
}

ECommonAvailability UCurrentSaveGameViewModel::GetTimeSinceLastSave(int32& OutHours, int32& OutMinutes, int32& OutSeconds) const
{
	if (!bHasTimeOfLastSave)
		return ECommonAvailability::Unavailable;

	const FDateTime CurrentUtcTime = FDateTime::UtcNow();
	const FTimespan TimeSinceLastSave = CurrentUtcTime - UtcTimeOfLastSave;
	OutHours = FMath::FloorToInt32(TimeSinceLastSave.GetTotalHours());
	OutMinutes = TimeSinceLastSave.GetMinutes();
	OutSeconds = TimeSinceLastSave.GetSeconds();
	return ECommonAvailability::Available;
}

void UCurrentSaveGameViewModel::BeginDestroy()
{
	EndUsage();

	Super::BeginDestroy();
}

void UCurrentSaveGameViewModel::UpdateForCurrentSaveGame(const FCurrentSaveGame& CurrentSaveGame)
{
	UE_MVVM_SET_PROPERTY_VALUE(bCanContinue, (CurrentSaveGame.IsValid() && !CurrentSaveGame.IsNewGame()));
	UpdateTimeSinceLastSave();
}

void UCurrentSaveGameViewModel::UpdateTimeSinceLastSave()
{
	if (TOptional<FDateTime> Timestamp = SaveGameService->GetCurrentSaveGame().GetUtcTimeOfLastSave(); Timestamp.IsSet())
	{
		UE_MVVM_SET_PROPERTY_VALUE(bHasTimeOfLastSave, true);
		UE_MVVM_SET_PROPERTY_VALUE(UtcTimeOfLastSave, *Timestamp);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetTimespanSinceLastSave);
	}
	else
	{
		UE_MVVM_SET_PROPERTY_VALUE(bHasTimeOfLastSave, false);
	}
}

void UCurrentSaveGameViewModel::EndUsage()
{
	if (SaveGameService)
	{
		SaveGameService->OnAfterRestored.RemoveAll(this);
		SaveGameService->OnAvailableSaveGamesChanged.RemoveAll(this);
		SaveGameService = nullptr;
	}
}
