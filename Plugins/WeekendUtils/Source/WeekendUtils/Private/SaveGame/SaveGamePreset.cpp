///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "SaveGame/SaveGamePreset.h"

#include "AssetRegistry/IAssetRegistry.h"
#include "GameFramework/SaveGame.h"
#include "SaveGame/ModularSaveGame.h"
#include "SaveGame/SaveGameHeader.h"
#include "SaveGame/SaveGameService.h"
#include "SaveGame/Settings/SaveGameServiceSettings.h"

#if WITH_EDITORONLY_DATA
#include "EditorUtilityLibrary.h"
#include "Misc/DataValidation.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogSaveGamePreset, Log, All);

///////////////////////////////////////////////////////////////////////////////////////

namespace
{
	void DiscoverPresets()
	{
		if (IAssetRegistry* AssetRegistry = IAssetRegistry::Get())
		{
			const USaveGameServiceSettings* Settings = GetDefault<USaveGameServiceSettings>();
			const FString DefaultFolder = Settings->DefaultSaveGamePresetFolder.Path;
			AssetRegistry->ScanPathsSynchronous({DefaultFolder});

			TArray<FAssetData> SaveGamePresetAssets;
			AssetRegistry->GetAssetsByClass(USaveGamePreset::StaticClass()->GetClassPathName(), OUT SaveGamePresetAssets, true);
			for (const FAssetData& AssetData : SaveGamePresetAssets)
			{
				LoadPackage(nullptr, *AssetData.PackageName.ToString(), LOAD_Quiet);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////

USaveGamePreset::USaveGamePreset()
{
	HeaderData = FInstancedStruct::Make<FSimpleSaveGameHeaderData>();
	SaveGame = CreateDefaultSubobject<UModularSaveGame>("ModularSaveGame");
}

void USaveGamePreset::OpenSaveGamePresetsFolder()
{
#if WITH_EDITOR
	const FString PresetsFolder = GetDefault<USaveGameServiceSettings>()->DefaultSaveGamePresetFolder.Path;
	UEditorUtilityLibrary::SyncBrowserToFolders({PresetsFolder});
#else
	unimplemented();
#endif
}

TSet<const USaveGamePreset*> USaveGamePreset::CollectSaveGamePresets()
{
	TSet<FSlotName> OccupiedSlotNames = {};
	TSet<const USaveGamePreset*> Result = {};

	DiscoverPresets();
	for (const USaveGamePreset* Preset : TObjectRange<USaveGamePreset>())
	{
		if (!Preset || !Preset->SaveGame)
			continue;

#if !WITH_EDITOR
		if (Preset->bIsEditorOnly)
			continue;
#endif

#if (UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if (Preset->bIsDeveloperOnly)
			continue;
#endif

		if (OccupiedSlotNames.Contains(Preset->PresetName))
		{
			UE_LOG(LogSaveGamePreset, Error, TEXT("Multiple SaveGamePreset assets use the same SlotName: %s"), *Preset->PresetName);
			continue;
		}

		OccupiedSlotNames.Add(Preset->PresetName);
		Result.Add(Preset);
	}

	return Result;
}

TSet<USaveGamePreset::FSlotName> USaveGamePreset::CollectSaveGamePresetNames()
{
	TSet<FSlotName> Result = {};

	DiscoverPresets();
	for (const USaveGamePreset* Preset : TObjectRange<USaveGamePreset>())
	{
		if (!Preset || !Preset->SaveGame)
			continue;

#if !WITH_EDITOR
		if (Preset->bIsEditorOnly)
			continue;
#endif

#if (UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if (Preset->bIsDeveloperOnly)
			continue;
#endif

		Result.Add(Preset->PresetName);
	}

	return Result;
}

const USaveGamePreset* USaveGamePreset::FindSaveGamePreset(const FSlotName& PresetName)
{
	DiscoverPresets();
	for (const USaveGamePreset* Preset : TObjectRange<USaveGamePreset>())
	{
		if (Preset && Preset->PresetName == PresetName)
			return Preset;
	}

	return nullptr;
}

void USaveGamePreset::RestoreAsCurrentSaveGame(USaveGameService& SaveGameService) const
{
	// Create a new save game instance, using the instanced one configured in the preset as template:
	USaveGame* SaveGameObject = CreateSaveGameObject(SaveGameService);
	SaveGameService.RestoreAsCurrentSaveGame(*SaveGameObject, PresetName);
}

void USaveGamePreset::RestoreAsAndTravelIntoCurrentSaveGame(USaveGameService& SaveGameService) const
{
	// Create a new save game instance, using the instanced one configured in the preset as template:
	USaveGame* SaveGameObject = CreateSaveGameObject(SaveGameService);
	SaveGameService.RestoreAsAndTravelIntoCurrentSaveGame(*SaveGameObject, PresetName);
}

USaveGame* USaveGamePreset::CreateSaveGameObject(USaveGameService& SaveGameService) const
{
	check(SaveGame);
	const FName SaveGameObjectName = MakeUniqueObjectName(&SaveGameService, SaveGame->GetClass(), FName("Preset"));
	USaveGame* ActualSaveGameObject = NewObject<USaveGame>(&SaveGameService, SaveGame->GetClass(), SaveGameObjectName, RF_NoFlags, static_cast<USaveGame*>(SaveGame));

	UModularSaveGame* ModularSaveGame = Cast<UModularSaveGame>(ActualSaveGameObject);
	if (const FSimpleSaveGameHeaderData* SimpleHeaderData = HeaderData.GetPtr<FSimpleSaveGameHeaderData>(); ModularSaveGame && SimpleHeaderData)
	{
		ModularSaveGame->CreateHeaderData(*SimpleHeaderData);
	}

	return ActualSaveGameObject;
}

#if WITH_EDITOR
EDataValidationResult USaveGamePreset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	if (PresetName.IsEmpty())
	{
		Context.AddError(INVTEXT("SlotName is not set. Should be a unique name across all other presets!"));
		Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);
	}
	if (!SaveGame)
	{
		Context.AddError(INVTEXT("SaveGame is invalid."));
		Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);
	}
	else
	{
		// Also check the SavGame for errors, but use the const version of IsDataValid to avoid the deprecation warnings:
		Result = CombineDataValidationResults(Result, TObjectPtr<const USaveGame>(SaveGame)->IsDataValid(Context));
	}
	return Result;
}
#endif
