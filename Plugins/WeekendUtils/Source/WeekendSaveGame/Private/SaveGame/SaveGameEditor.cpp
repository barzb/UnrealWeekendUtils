///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "SaveGame/SaveGameEditor.h"

#if WITH_EDITORONLY_DATA

#include "Editor.h"
#include "Factories/DataAssetFactory.h"
#include "GameService/GameServiceLocator.h"
#include "IAssetTools.h"
#include "SaveGame/ModularSaveGame.h"
#include "SaveGame/SaveGamePreset.h"
#include "SaveGame/SaveGameService.h"
#include "SaveGame/Settings/SaveGameServiceSettings.h"
#include "Subsystems/AssetEditorSubsystem.h"

namespace
{
	TStrongObjectPtr<USaveGameEditor> ShownInstance = nullptr;
}

void USaveGameEditor::OpenSaveGameEditor(const USaveGame* SaveGame)
{
	if (!ShownInstance.IsValid())
	{
		// (i) Need to be kept alive or the editor crashes on GC.
		ShownInstance = TStrongObjectPtr(NewObject<USaveGameEditor>(GetTransientPackage(), FName("Savegame Editor")));
	}

	ShownInstance->SetSaveGame(SaveGame);

	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	AssetEditorSubsystem->OpenEditorForAsset<USaveGameEditor>(ShownInstance.Get());
}

void USaveGameEditor::OpenSaveGameEditorForCurrentSaveGame()
{
	const USaveGame* SaveGame = nullptr;
	if (const USaveGameService* SaveGameService = UGameServiceLocator::FindService<USaveGameService>())
	{
		const FCurrentSaveGame& CurrentSaveGame = SaveGameService->GetCurrentSaveGame();
		SaveGame = CurrentSaveGame.GetPtr();
	}
	OpenSaveGameEditor(SaveGame);
}

void USaveGameEditor::ConvertToPreset()
{
	if (!SaveGame)
		return;

	const USaveGameServiceSettings* Settings = GetDefault<USaveGameServiceSettings>();
	const FString DefaultFolder = Settings->DefaultSaveGamePresetFolder.Path;
	const FString AssetPrefix = Settings->DefaultSaveGamePresetAssetPrefix;

	UDataAssetFactory* PresetFactory = NewObject<UDataAssetFactory>();
	PresetFactory->DataAssetClass = USaveGamePreset::StaticClass();
	if (UObject* CreatedAsset = IAssetTools::Get().CreateAssetWithDialog(
		AssetPrefix, DefaultFolder, USaveGamePreset::StaticClass(), PresetFactory, NAME_None, false))
	{
		USaveGamePreset* CreatedPreset = Cast<USaveGamePreset>(CreatedAsset);
		CreatedPreset->HeaderData = HeaderData;
		CreatedPreset->SaveGame = DuplicateObject<USaveGame>(SaveGame, CreatedAsset);

		const FString AssetNameWithoutPrefix = CreatedAsset->GetName().Replace(*AssetPrefix, TEXT(""));
		CreatedPreset->PresetName = AssetNameWithoutPrefix;

		UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
		AssetEditorSubsystem->OpenEditorForAsset<USaveGamePreset>(CreatedPreset);
	}
}

void USaveGameEditor::EditCurrentSaveGame()
{
	if (const USaveGameService* SaveGameService = UGameServiceLocator::FindService<USaveGameService>())
	{
		const FCurrentSaveGame& CurrentSaveGame = SaveGameService->GetCurrentSaveGame();
		SetSaveGame(CurrentSaveGame.GetPtr());
	}
	else
	{
		SetSaveGame(nullptr);
	}
}

void USaveGameEditor::SetSaveGame(const USaveGame* InSaveGame)
{
	SaveGame = InSaveGame;
	if (const UModularSaveGame* ModularSaveGame = Cast<UModularSaveGame>(SaveGame))
	{
		ShownInstance->HeaderData = ModularSaveGame->GetInstancedHeaderData().IsValid()
			? *ModularSaveGame->GetInstancedHeaderData().Get()
			: FInstancedStruct();
	}

	EditorInfo = FString("Editing: ") + GetNameSafe(SaveGame);
}

#else

void USaveGameEditor::OpenSaveGameEditor(const USaveGame*) { unimplemented(); }
void USaveGameEditor::OpenSaveGameEditorForCurrentSaveGame() { unimplemented(); }
void USaveGameEditor::ConvertToPreset() { unimplemented(); }
void USaveGameEditor::EditCurrentSaveGame() { unimplemented(); }

#endif
