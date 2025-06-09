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
#include "Kismet/BlueprintFunctionLibrary.h"

#include "SaveGameUtils.generated.h"

UCLASS()
class WEEKENDSAVEGAME_API USaveGameUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Weekend Utils|Save Game|Editor", meta = (DevelopmentOnly))
	static void OpenSaveGameProjectSettings();

	UFUNCTION(BlueprintCallable, Category = "Weekend Utils|Save Game|Editor", meta = (DevelopmentOnly))
	static void GetOverridePlayInEditorSaveGameSlot(bool& bOutIsOverridden, FString& OutSlotName);

	UFUNCTION(BlueprintCallable, Category = "Weekend Utils|Save Game|Editor", meta = (DevelopmentOnly))
	static void SetOverridePlayInEditorSaveGameSlot(bool bOverride, FString SlotName);

	UFUNCTION(BlueprintCallable, Category = "Weekend Utils|Save Game|Editor", meta = (DevelopmentOnly))
	static TArray<FString> FindAllSaveGamePresetNames();

	UFUNCTION(BlueprintCallable, Category = "Weekend Utils|Save Game")
	static TArray<FString> FindAllLocalSaveGameSlotNames();

	UFUNCTION(BlueprintCallable, Category = "Weekend Utils|Save Game")
	static void DeleteAllLocalSaveGames(int32 UserIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "Weekend Utils|Save Game")
	static bool IsSavingAllowedForWorld(UWorld* World);
};
