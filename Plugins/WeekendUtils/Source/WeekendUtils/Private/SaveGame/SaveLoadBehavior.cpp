///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "SaveGame/SaveLoadBehavior.h"

#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "SaveGame/ModularSaveGame.h"
#include "SaveGame/SaveGamePreset.h"
#include "SaveGame/SaveGameService.h"
#include "SaveGame/SaveGameUtils.h"
#include "SaveGame/Settings/SaveGameServiceSettings.h"

DEFINE_LOG_CATEGORY(LogSaveLoadBehavior);

///////////////////////////////////////////////////////////////////////////////////////
/// UTILS

namespace
{
	FSimpleSaveGameHeaderData* TryGetHeaderData(const FCurrentSaveGame& SaveGame)
	{
		const UModularSaveGame* ModularSaveGame = SaveGame.GetPtr<UModularSaveGame>();
		if (!ModularSaveGame)
			return nullptr;

		return ModularSaveGame->GetMutableHeaderDataPtr<FSimpleSaveGameHeaderData>();
	}

	FSimpleSaveGameHeaderData* TryGetHeaderData(const USaveGame& SaveGame)
	{
		const UModularSaveGame* ModularSaveGame = Cast<UModularSaveGame>(&SaveGame);
		if (!ModularSaveGame)
			return nullptr;

		return ModularSaveGame->GetMutableHeaderDataPtr<FSimpleSaveGameHeaderData>();
	}
}

///////////////////////////////////////////////////////////////////////////////////////
/// @USaveLoadBehavior

void USaveLoadBehavior::HandleBeforeSaveGameSaved(const FCurrentSaveGame& SaveGame, USaveGameService* SaveGameService)
{
	if (FSimpleSaveGameHeaderData* HeaderData = TryGetHeaderData(SaveGame))
	{
		HeaderData->SaveCounter++;
		HeaderData->UtcTimeOfLastSave = FDateTime::UtcNow();
		HeaderData->LoadedLevel = FSoftObjectPath(GetWorld()->GetPathName());
	}
}

void USaveLoadBehavior::HandleAfterSaveGameRestored(const FCurrentSaveGame& SaveGame, USaveGameService* SaveGameService)
{
	if (FSimpleSaveGameHeaderData* HeaderData = TryGetHeaderData(SaveGame))
	{
		HeaderData->RestoreCounter++;
		HeaderData->UtcTimeOfLastRestore = FDateTime::UtcNow();
	}
}

USaveGame& USaveLoadBehavior::CreateNewSavegameObject(USaveGameService& SaveGameService) const
{
	return *NewObject<UModularSaveGame>(&SaveGameService);
}

TSubclassOf<USaveGameSerializer> USaveLoadBehavior::GetSaveGameSerializerClass() const
{
	return UModularSaveGameSerializer::StaticClass();
}

TOptional<FDateTime> USaveLoadBehavior::FindTimeOfLastSaveFromSaveGame(const USaveGame& SaveGame) const
{
	if (const FSimpleSaveGameHeaderData* HeaderData = TryGetHeaderData(SaveGame); (HeaderData && HeaderData->WasEverSaved()))
	{
		return HeaderData->UtcTimeOfLastSave;
	}

	return {};
}

bool USaveLoadBehavior::TryTravelToSavedLevel(const FCurrentSaveGame& SaveGame)
{
	const FSimpleSaveGameHeaderData* HeaderData = TryGetHeaderData(SaveGame);
	if (!HeaderData)
	{
		UE_LOG(LogSaveLoadBehavior, Error, TEXT("Cannot travel to saved level (%s): Unexpected SaveGame structure"), *GetNameSafe(SaveGame.GetPtr()));
		return false;
	}

	if (!HeaderData->LoadedLevel.IsValid())
	{
		UE_LOG(LogSaveLoadBehavior, Error, TEXT("Cannot travel to saved level (%s): LoadedLevel is invalid"), *GetNameSafe(SaveGame.GetPtr()));
		return false;
	}

	const FName LevelToLoad = HeaderData->LoadedLevel.GetLongPackageFName();
	UGameplayStatics::OpenLevel(this, LevelToLoad);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////
/// @UDefaultSaveLoadBehavior

UDefaultSaveLoadBehavior::UDefaultSaveLoadBehavior()
{
	SaveSlotNames =
	{
		"GameSlot1", "GameSlot2", "GameSlot3", "GameSlot4",
		"GameSlot5", "GameSlot6", "GameSlot7", "GameSlot8",
	};
}

void UDefaultSaveLoadBehavior::HandleGameStart(USaveGameService& SaveGameService)
{
	SaveGameService.PreloadSaveGamesAsync(GetSaveSlotNamesAllowedForLoading(), FOnPreloadCompleted::CreateWeakLambda(this,
		[this, SaveGameService = MakeWeakObjectPtr(&SaveGameService)](const TSet<USaveGame*>& PreloadedSaveGames)
		{
			if (!SaveGameService.IsValid())
				return;

			HandlePreloadCompleted(*SaveGameService, PreloadedSaveGames);
		}));
}

TSet<USaveLoadBehavior::FSlotName> UDefaultSaveLoadBehavior::GetSaveSlotNamesAllowedForSaving() const
{
	return SaveSlotNames;
}

TSet<USaveLoadBehavior::FSlotName> UDefaultSaveLoadBehavior::GetSaveSlotNamesAllowedForLoading() const
{
	TSet<FSlotName> Result = SaveSlotNames;
	Result.Add(GetAutosaveSlotName());
	return Result;
}

void UDefaultSaveLoadBehavior::HandlePreloadCompleted(USaveGameService& SaveGameService, const TSet<USaveGame*>& SaveGames)
{
	// Attempt to find and restore the most "recent" save game:

	TOptional<FDateTime> MostRecentSaveTime = {};
	TOptional<USaveGame*> MostRecentSaveGame = {};
	for (USaveGame* SaveGame : SaveGames)
	{
		if (!SaveGame)
			continue;

		TOptional<FDateTime> TimeOfLastSave = FindTimeOfLastSaveFromSaveGame(*SaveGame);
		if (!TimeOfLastSave.IsSet())
			continue;

		if (!MostRecentSaveTime.IsSet() || ((*TimeOfLastSave) > (*MostRecentSaveTime)))
		{
			MostRecentSaveTime = TimeOfLastSave;
			MostRecentSaveGame = SaveGame;
		}
	}

	if (MostRecentSaveGame.IsSet())
	{
		UE_LOG(LogSaveLoadBehavior, Log, TEXT("Preload completed. Loading most recent save game (%s), saved on (UTC) %s"),
			*GetNameSafe(*MostRecentSaveGame), *MostRecentSaveTime->ToString());
		SaveGameService.RestoreAsCurrentSaveGame(**MostRecentSaveGame);
	}
}

///////////////////////////////////////////////////////////////////////////////////////
/// @UDefaultPlayInEditorSaveLoadBehavior

void UDefaultPlayInEditorSaveLoadBehavior::HandleGameStart(USaveGameService& SaveGameService)
{
#if WITH_EDITOR
	const FSlotName SlotName = GetPlayInEditorSlotName();
	if (SlotName.IsEmpty())
		return;

	if (SaveGameService.DoesSaveFileExist(SlotName))
	{
		SaveGameService.TryLoadCurrentSaveGameFromSlotSynchronous(SlotName);
	}
	else if (const USaveGamePreset* Preset = USaveGamePreset::FindSaveGamePreset(SlotName))
	{
		Preset->RestoreAsCurrentSaveGame(SaveGameService);
	}
#else
	unimplemented();
#endif
}

TSet<USaveLoadBehavior::FSlotName> UDefaultPlayInEditorSaveLoadBehavior::GetSaveSlotNamesAllowedForSaving() const
{
	return { GetPlayInEditorSlotName() };
}

TSet<USaveLoadBehavior::FSlotName> UDefaultPlayInEditorSaveLoadBehavior::GetSaveSlotNamesAllowedForLoading() const
{
	return { GetPlayInEditorSlotName() };
}

USaveLoadBehavior::FSlotName UDefaultPlayInEditorSaveLoadBehavior::GetAutosaveSlotName() const
{
	return GetPlayInEditorSlotName();
}

USaveLoadBehavior::FSlotName UDefaultPlayInEditorSaveLoadBehavior::GetPlayInEditorSlotName() const
{
	bool bUseOverrideSlot = false;
	FSlotName OverrideSlotName = "";
	USaveGameUtils::GetOverridePlayInEditorSaveGameSlot(OUT bUseOverrideSlot, OUT OverrideSlotName);
	return (bUseOverrideSlot ? OverrideSlotName : GetDefault<USaveGameServiceSettings>()->DefaultPlayInEditorSaveGameSlotName);
}
