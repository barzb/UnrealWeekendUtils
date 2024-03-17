///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "SaveGame/ViewModels/SaveGameList_VM.h"

#include "SaveGame/SaveGamePreset.h"
#include "SaveGame/SaveGameService.h"
#include "SaveGame/ViewModels/SaveGameSlot_VM.h"
#include "Utils/ObjectListSynchronizer.h"

USaveGameList_VM::USaveGameList_VM()
{
	ServiceDependencies.Add<USaveGameService>();
}

void USaveGameList_VM::BeginUsage(TSubclassOf<USaveGameSlot_VM> SlotClass)
{
	SlotViewModelClass = SlotClass;

	SaveGameService = UseGameServiceAsWeakPtr<USaveGameService>(this);
	SaveGameService->OnAvailableSaveGamesChanged.AddUObject(this, &ThisClass::Update);

	Update();
}

void USaveGameList_VM::Update()
{
	const TArray<FSlotName> SlotsNames = GatherRelevantSlotNames();
	const bool bCanSave = AllowsSavingFromWidget();
	const bool bCanLoad = AllowsLoadingFromWidget();

	TObjectListSynchronizer(Slots, SlotsNames)
	.ForEachMissingElement([this, bCanSave, bCanLoad](const FSlotName& SlotName) -> USaveGameSlot_VM* {
		USaveGameSlot_VM* NewViewModel = NewObject<USaveGameSlot_VM>(this, SlotViewModelClass);
		NewViewModel->OnSaveRequested.BindUObject(this, &ThisClass::HandleSaveRequestBySlot);
		NewViewModel->OnLoadRequested.BindUObject(this, &ThisClass::HandleLoadRequestBySlot);
		NewViewModel->BindToModel(SlotName, *SaveGameService, bCanSave, bCanLoad);
		return NewViewModel;
	})
	.ForEachUpdatedElement([this, bCanSave, bCanLoad](USaveGameSlot_VM& ViewModel, const FSlotName& SlotName)
	{
		ViewModel.BindToModel(SlotName, *SaveGameService, bCanSave, bCanLoad);
	})
	.ForEachRemovedElement([this](USaveGameSlot_VM& ViewModel)
	{
		ViewModel.OnSaveRequested.Unbind();
		ViewModel.OnLoadRequested.Unbind();
		ViewModel.UnbindFromModel();
	});

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Slots);
}

void USaveGameList_VM::EndUsage()
{
	Slots.Reset();

	if (SaveGameService.IsValid())
	{
		SaveGameService->OnAvailableSaveGamesChanged.RemoveAll(this);
		SaveGameService.Reset();
	}
}

void USaveGameList_VM::BeginDestroy()
{
	EndUsage();

	Super::BeginDestroy();
}

///////////////////////////////////////////////////////////////////////////////////////

TArray<USaveGameList_VM::FSlotName> USaveGameSaveList_VM::GatherRelevantSlotNames()
{
	if (!SaveGameService.IsValid())
		return {};

	return SaveGameService->GetSlotNamesAllowedForSaving().Array();
}

bool USaveGameSaveList_VM::HandleSaveRequestBySlot(const FSlotName& SlotName)
{
	if (!SaveGameService.IsValid())
		return false;

	SaveGameService->RequestSaveCurrentSaveGameToSlot(SlotName);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////

TArray<USaveGameList_VM::FSlotName> USaveGameLoadList_VM::GatherRelevantSlotNames()
{
	if (!SaveGameService.IsValid())
		return {};

	return SaveGameService->GetSlotNamesAllowedForLoading().Array();
}

bool USaveGameLoadList_VM::HandleLoadRequestBySlot(const FSlotName& SlotName)
{
	if (!SaveGameService.IsValid())
		return false;

	SaveGameService->RequestLoadAndTravelIntoCurrentSaveGameFromSlot(SlotName);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////

void USaveGamePresetList_VM::Update()
{
	AvailablePresets = USaveGamePreset::CollectSaveGamePresets().Array();

	Super::Update();
}

TArray<USaveGameList_VM::FSlotName> USaveGamePresetList_VM::GatherRelevantSlotNames()
{
	TArray<FSlotName> SlotNames = {};
	Algo::Transform(AvailablePresets, OUT SlotNames, &USaveGamePreset::PresetName);
	return SlotNames;
}

bool USaveGamePresetList_VM::HandleLoadRequestBySlot(const FSlotName& SlotName)
{
	if (!SaveGameService.IsValid())
		return false;

	const auto* Preset = Algo::FindBy(AvailablePresets, SlotName, &USaveGamePreset::PresetName);
	if (!Preset || !(*Preset))
		return false;

	(*Preset)->RestoreAsAndTravelIntoCurrentSaveGame(*SaveGameService.Get());
	return true;
}
