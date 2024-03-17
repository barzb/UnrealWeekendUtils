///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "SaveGame/ViewModels/SaveGameSlot_VM.h"

#include "SaveGame/SaveGameService.h"

void USaveGameSlot_VM::BindToModel(const FSlotName& SlotName, USaveGameService& SaveGameService, bool bCanSave, bool bCanLoad)
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

bool USaveGameSlot_VM::TryLoadGameFromSlot()
{
	return (OnLoadRequested.IsBound() && OnLoadRequested.Execute(BoundSlotName));
}

bool USaveGameSlot_VM::TrySaveGameToSlot()
{
	return (OnSaveRequested.IsBound() && OnSaveRequested.Execute(BoundSlotName));
}
