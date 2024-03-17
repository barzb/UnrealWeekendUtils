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
#include "MVVMViewModelBase.h"

#include "SaveGameSlot_VM.generated.h"

class USaveGame;
class USaveGameService;

/**
 * Base class of a SaveGame slot ViewModel that acts as list element of @USaveGameList_VM.
 */
UCLASS(Abstract)
class WEEKENDUTILS_API USaveGameSlot_VM : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	using FSlotName = FString;

	/** Events intended for @USaveGameList_VM to listen to. */
	DECLARE_DELEGATE_RetVal_OneParam(bool, FOnSaveLoadRequested, const FSlotName&)
	FOnSaveLoadRequested OnLoadRequested;
	FOnSaveLoadRequested OnSaveRequested;

	UPROPERTY(FieldNotify, BlueprintReadOnly)
	bool bCanBeSavedFromWidget = false;

	UPROPERTY(FieldNotify, BlueprintReadOnly)
	bool bCanBeLoadedFromWidget = false;

	UPROPERTY(FieldNotify, BlueprintReadOnly)
	bool bIsCurrentSaveGame = false;

	virtual void BindToModel(const FSlotName& SlotName, USaveGameService& SaveGameService, bool bCanSave, bool bCanLoad);
	virtual void BindToSaveGame(const FSlotName& SlotName, const USaveGame& SaveGame) PURE_VIRTUAL(BindToSaveGame);
	virtual void BindToEmptySlot(const FSlotName& SlotName) PURE_VIRTUAL(BindToEmptySlot);
	virtual void UnbindFromModel() PURE_VIRTUAL(UnbindFromModel);

protected:
	FSlotName BoundSlotName = FSlotName();

	UFUNCTION(BlueprintCallable)
	bool TryLoadGameFromSlot();

	UFUNCTION(BlueprintCallable)
	bool TrySaveGameToSlot();
};
