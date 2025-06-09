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
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"

#include "SaveGameEditor.generated.h"

class USaveGame;

/**
 * [Editor Only] Container object that can show exposed properties of a SaveGame object.
 */
UCLASS()
class WEEKENDSAVEGAME_API USaveGameEditor : public UObject
{
	GENERATED_BODY()

public:
	/** Opens the SaveGameEditor window for passed SaveGame object. */
	UFUNCTION(BlueprintCallable, Category = "Weekend Utils|Save Game|Editor", meta = (DevelopmentOnly))
	static void OpenSaveGameEditor(const USaveGame* SaveGame);

	/** Opens the SaveGameEditor window for the currently active SaveGame object. */
	UFUNCTION(BlueprintCallable, Category = "Weekend Utils|Save Game|Editor", meta = (DevelopmentOnly))
	static void OpenSaveGameEditorForCurrentSaveGame();

protected:
	// - UObject
	virtual bool IsEditorOnly() const override { return true; }
	// --

	/** Creates a new @USaveGamePreset data asset based on the currently edited SaveGame. */
	UFUNCTION(CallInEditor, Category = "Editor", meta = (DevelopmentOnly))
	virtual void ConvertToPreset();

	/** Attempts to show the actively loaded SaveGame in this editor window. Game must be running. */
	UFUNCTION(CallInEditor, Category = "Editor", meta = (DevelopmentOnly))
	virtual void EditCurrentSaveGame();

#if WITH_EDITORONLY_DATA
	virtual void SetSaveGame(const USaveGame* InSaveGame);

	UPROPERTY(VisibleAnywhere, Transient, Category = "Editor")
	FString EditorInfo = FString();

	UPROPERTY(Instanced, VisibleAnywhere, Transient, Category = "Save Game", meta = (ShowOnlyInnerProperties))
	TObjectPtr<const USaveGame> SaveGame = nullptr;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Save Game")
	FInstancedStruct HeaderData = FInstancedStruct();
#endif
};
