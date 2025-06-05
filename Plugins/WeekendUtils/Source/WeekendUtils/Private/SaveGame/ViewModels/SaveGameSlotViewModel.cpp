///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "SaveGame/ViewModels/SaveGameSlotViewModel.h"

#include "SaveGame/SaveGameService.h"

void USaveGameSlotViewModel::BindToModel(const FSlotName& SlotName, USaveGameService& SaveGameService, bool bCanSave, bool bCanLoad)
{
	BoundSlotName = SlotName;
	if (const USaveGame* SaveGame = SaveGameService.GetCachedSaveGameAtSlot(SlotName))
	{
		BindToSaveGame(SlotName, *SaveGame);
	}
	else
	{
		BindToEmptySlot(SlotName);
	}

	const FSlotName CurrentSlotName = SaveGameService.GetCurrentSaveGame().GetSlotLastRestoredFrom().Get("");
	UE_MVVM_SET_PROPERTY_VALUE(bIsCurrentSaveGame, (CurrentSlotName == SlotName));

	UE_MVVM_SET_PROPERTY_VALUE(bCanBeSavedFromWidget, bCanSave);
	UE_MVVM_SET_PROPERTY_VALUE(bCanBeLoadedFromWidget, bCanLoad);
}

bool USaveGameSlotViewModel::TryLoadGameFromSlot()
{
	return (OnLoadRequested.IsBound() && OnLoadRequested.Execute(BoundSlotName));
}

bool USaveGameSlotViewModel::TrySaveGameToSlot()
{
	return (OnSaveRequested.IsBound() && OnSaveRequested.Execute(BoundSlotName));
}
