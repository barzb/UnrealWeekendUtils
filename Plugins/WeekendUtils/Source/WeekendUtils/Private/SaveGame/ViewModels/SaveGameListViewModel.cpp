///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "SaveGame/ViewModels/SaveGameListViewModel.h"

#include "SaveGame/SaveGamePreset.h"
#include "SaveGame/SaveGameService.h"
#include "SaveGame/ViewModels/SaveGameSlotViewModel.h"
#include "Utils/ObjectListSynchronizer.h"

USaveGameListViewModel::USaveGameListViewModel()
{
	ServiceDependencies.Add<USaveGameService>();
}

void USaveGameListViewModel::BeginUsage(TSubclassOf<USaveGameSlotViewModel> SlotClass)
{
	SlotViewModelClass = SlotClass;
	check(SlotViewModelClass);

	SaveGameService = UseGameServiceAsWeakPtr<USaveGameService>(this);
	SaveGameService->OnAvailableSaveGamesChanged.AddUObject(this, &ThisClass::Update);

	Update();
}

void USaveGameListViewModel::Update()
{
	const TArray<FSlotName> SlotsNames = GatherRelevantSlotNames();
	const bool bCanSave = AllowsSavingFromWidget();
	const bool bCanLoad = AllowsLoadingFromWidget();

	TObjectListSynchronizer(Slots, SlotsNames)
	.ForEachMissingElement([this, bCanSave, bCanLoad](const FSlotName& SlotName) -> USaveGameSlotViewModel* {
		USaveGameSlotViewModel* NewViewModel = NewObject<USaveGameSlotViewModel>(this, SlotViewModelClass);
		NewViewModel->OnSaveRequested.BindUObject(this, &ThisClass::HandleSaveRequestBySlot);
		NewViewModel->OnLoadRequested.BindUObject(this, &ThisClass::HandleLoadRequestBySlot);
		NewViewModel->BindToModel(SlotName, *SaveGameService, bCanSave, bCanLoad);
		return NewViewModel;
	})
	.ForEachUpdatedElement([this, bCanSave, bCanLoad](USaveGameSlotViewModel& ViewModel, const FSlotName& SlotName)
	{
		ViewModel.BindToModel(SlotName, *SaveGameService, bCanSave, bCanLoad);
	})
	.ForEachRemovedElement([this](USaveGameSlotViewModel& ViewModel)
	{
		ViewModel.OnSaveRequested.Unbind();
		ViewModel.OnLoadRequested.Unbind();
		ViewModel.UnbindFromModel();
	});

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Slots);
}

void USaveGameListViewModel::EndUsage()
{
	Slots.Reset();

	if (SaveGameService.IsValid())
	{
		SaveGameService->OnAvailableSaveGamesChanged.RemoveAll(this);
		SaveGameService.Reset();
	}
}

void USaveGameListViewModel::BeginDestroy()
{
	EndUsage();

	Super::BeginDestroy();
}

///////////////////////////////////////////////////////////////////////////////////////

TArray<USaveGameListViewModel::FSlotName> USaveGameSaveListViewModel::GatherRelevantSlotNames()
{
	if (!SaveGameService.IsValid())
		return {};

	return SaveGameService->GetSlotNamesAllowedForSaving().Array();
}

bool USaveGameSaveListViewModel::HandleSaveRequestBySlot(const FSlotName& SlotName)
{
	if (!SaveGameService.IsValid())
		return false;

	SaveGameService->RequestSaveCurrentSaveGameToSlot("SaveList Entry (USaveGameSaveListViewModel)", SlotName);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////

TArray<USaveGameListViewModel::FSlotName> USaveGameLoadListViewModel::GatherRelevantSlotNames()
{
	if (!SaveGameService.IsValid())
		return {};

	return SaveGameService->GetSlotNamesAllowedForLoading().Array();
}

bool USaveGameLoadListViewModel::HandleLoadRequestBySlot(const FSlotName& SlotName)
{
	if (!SaveGameService.IsValid())
		return false;

	SaveGameService->RequestLoadAndTravelIntoCurrentSaveGameFromSlot("LoadList Entry (USaveGameLoadListViewModel)", SlotName);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////

void USaveGamePresetListViewModel::Update()
{
	AvailablePresets = USaveGamePreset::CollectSaveGamePresets().Array();

	Super::Update();
}

TArray<USaveGameListViewModel::FSlotName> USaveGamePresetListViewModel::GatherRelevantSlotNames()
{
	TArray<FSlotName> SlotNames = {};
	Algo::Transform(AvailablePresets, OUT SlotNames, &USaveGamePreset::PresetName);
	return SlotNames;
}

bool USaveGamePresetListViewModel::HandleLoadRequestBySlot(const FSlotName& SlotName)
{
	if (!SaveGameService.IsValid())
		return false;

	const auto* Preset = Algo::FindBy(AvailablePresets, SlotName, &USaveGamePreset::PresetName);
	if (!Preset || !(*Preset))
		return false;

	(*Preset)->RestoreAsAndTravelIntoCurrentSaveGame(*SaveGameService.Get());
	return true;
}
