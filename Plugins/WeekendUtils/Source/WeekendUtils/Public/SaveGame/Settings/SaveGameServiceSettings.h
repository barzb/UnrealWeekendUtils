///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SaveGame/Mocks/MockableSaveLoadBehavior.h"
#include "SaveGame/SaveLoadBehavior.h"

#include "SaveGameServiceSettings.generated.h"

/**
 * Project settings for the @USaveGameService and its surrounding API.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Savegame Service"))
class WEEKENDUTILS_API USaveGameServiceSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Behavior that defines which and when save games will be saved & loaded for the game. */
	UPROPERTY(Config, EditDefaultsOnly, NoClear, Category = "Save Load Behavior")
	TSoftClassPtr<USaveLoadBehavior> SaveLoadBehavior = UDefaultSaveLoadBehavior::StaticClass();

#if WITH_EDITORONLY_DATA
	/** Behavior that defines which and when save games will be saved & loaded for Play In Standalone (Editor). */
	UPROPERTY(Config, EditDefaultsOnly, NoClear, Category = "Save Load Behavior")
	TSoftClassPtr<USaveLoadBehavior> PlayInStandaloneSaveLoadBehavior = UDefaultSaveLoadBehavior::StaticClass();

	/** Behavior that defines which and when save games will be saved & loaded for Play In Editor. */
	UPROPERTY(Config, EditDefaultsOnly, NoClear, Category = "Save Load Behavior")
	TSoftClassPtr<USaveLoadBehavior> PlayInEditorSaveLoadBehavior = UDefaultPlayInEditorSaveLoadBehavior::StaticClass();
#endif

	/** Behavior that defines which and when save games will be saved & loaded for automation tests. */
	UPROPERTY(Config, EditDefaultsOnly, NoClear, Category = "Save Load Behavior")
	TSoftClassPtr<UMockableSaveLoadBehavior> AutomationTestSaveLoadBehavior = UMockableSaveLoadBehavior::StaticClass();

	/** When enabled, saving to a SaveGame is always allowed by default (if not locked manually through game logic). */
	UPROPERTY(Config, EditDefaultsOnly, Category = "Save Load Behavior")
	bool bAlwaysAllowSaving = true;

	/** Allow saving to a SaveGame while playing in these maps. */
	UPROPERTY(Config, EditDefaultsOnly, Category = "Save Load Behavior", meta = (EditCondition = "!bAlwaysAllowSaving", AllowedClasses = "/Script/Engine.World"))
	TSet<FSoftObjectPath> MapsWhereSavingIsAllowed = {};

	/** Allow saving to a SaveGame while playing in these GameModes. */
	UPROPERTY(Config, EditDefaultsOnly, Category = "Save Load Behavior", meta = (EditCondition = "!bAlwaysAllowSaving", MetaClass = "/Script/Engine.GameModeBase"))
	TSet<FSoftClassPath> GameModesWhereSavingIsAllowed = {};

	/** Name of the SaveGame slot to save to while playing in editor (see @UDefaultPlayInEditorSaveLoadBehavior). */
	UPROPERTY(Config, EditAnywhere, Category = "Play In Editor")
	FString DefaultPlayInEditorSaveGameSlotName = "PlayInEditor";

	/** Where the project stores @USaveGamePreset assets. */
	UPROPERTY(Config, EditDefaultsOnly, Category = "Save Game Presets", meta = (ContentDir))
	FDirectoryPath DefaultSaveGamePresetFolder = FDirectoryPath("/Game/Savegames/");

	/** Default prefix that is given to @USaveGamePreset assets when created via factory. */
	UPROPERTY(Config, EditDefaultsOnly, Category = "Save Game Presets")
	FString DefaultSaveGamePresetAssetPrefix = "DA_SaveGame_";
};
