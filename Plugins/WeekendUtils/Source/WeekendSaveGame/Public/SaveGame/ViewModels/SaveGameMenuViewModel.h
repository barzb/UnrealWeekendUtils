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
#include "CurrentSaveGameViewModel.h"

#include "SaveGameMenuViewModel.generated.h"

/**
 * ViewModel that represents not only the currently loaded SaveGame slot, but also provides information
 * about whether the game can be saved or loaded at the moment.
 */
UCLASS()
class WEEKENDSAVEGAME_API USaveGameMenuViewModel : public UCurrentSaveGameViewModel
{
	GENERATED_BODY()

public:
	/** @returns whether the button to open the menu for loading savegames should be shown.  */
	UFUNCTION(FieldNotify, BlueprintCallable, Category = "Weekend Utils|Save Game")
	bool ShouldShowLoadButton() const;

	/** @returns whether the button to open the menu for saving savegames should be shown.  */
	UFUNCTION(FieldNotify, BlueprintCallable, Category = "Weekend Utils|Save Game")
	bool ShouldShowSaveButton() const;

	// - UCurrentSaveGameViewModel
	virtual void BeginUsage() override;
	virtual void EndUsage() override;
	// --

protected:
	// - UCurrentSaveGameViewModel
	virtual void UpdateForCurrentSaveGame(const FCurrentSaveGame& CurrentSaveGame) override;
	// --

	virtual void UpdateSaveLoadButtonAvailability();
};
