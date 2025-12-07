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
#include "Engine/DataAsset.h"
#include "StructUtils/InstancedStruct.h"

#include "SaveGamePreset.generated.h"

class USaveGame;
class USaveGameService;

/**
 * Asset that instances a SaveGame object and optional HeaderData.
 * Presets can be loaded like serialized SaveGame files to restore a game.
 */
UCLASS(BlueprintType, CollapseCategories)
class WEEKENDSAVEGAME_API USaveGamePreset : public UDataAsset
{
	GENERATED_BODY()

public:
	using FSlotName = FString;

	UPROPERTY(EditDefaultsOnly, Category = "Weekend Utils|Save Game")
	bool bIsEditorOnly = false;

	UPROPERTY(EditDefaultsOnly, Category = "Weekend Utils|Save Game")
	bool bIsDeveloperOnly = true;

	/** Pretends to be a SaveGame slot, so it should be unique across other presets. */
	UPROPERTY(EditDefaultsOnly, NoClear, Category = "Weekend Utils|Save Game")
	FString PresetName = FString();

	UPROPERTY(EditDefaultsOnly, meta = (ExcludeBaseStruct, BaseStruct = "/Script/WeekendUtils.SaveGameHeaderDataBase"), Category = "Weekend Utils|Save Game")
	FInstancedStruct HeaderData;

	UPROPERTY(Instanced, EditDefaultsOnly, NoClear, meta = (ShowOnlyInnerProperties), Category = "Weekend Utils|Save Game")
	TObjectPtr<const USaveGame> SaveGame;

	USaveGamePreset();

	UFUNCTION(BlueprintCallable, Category = "Weekend Utils|Save Game", meta = (DevelopmentOnly))
	static void OpenSaveGamePresetsFolder();

	/** Scans for available presets that match the current build environment. */
	static TSet<const USaveGamePreset*> CollectSaveGamePresets();

	/** Scans for available presets that match the current build environment. */
	static TSet<FSlotName> CollectSaveGamePresetNames();

	/** @returns and scans a particular preset. */
	static const USaveGamePreset* FindSaveGamePreset(const FSlotName& PresetName);

	virtual void RestoreAsCurrentSaveGame(USaveGameService& SaveGameService) const;
	virtual void RestoreAsAndTravelIntoCurrentSaveGame(USaveGameService& SaveGameService) const;
	virtual USaveGame* CreateSaveGameObject(USaveGameService& SaveGameService) const;

	// - UObject
	virtual bool IsEditorOnly() const override { return bIsEditorOnly; }
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	// --
};
