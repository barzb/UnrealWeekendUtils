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
#include "GameService/GameServiceUser.h"

#include "CurrentSaveGameViewModel.generated.h"

struct FCurrentSaveGame;

/**
 * ViewModel that represents the currently loaded SaveGame slot.
 */
UCLASS()
class WEEKENDSAVEGAME_API UCurrentSaveGameViewModel : public UMVVMViewModelBase,
													  public FGameServiceUser
{
	GENERATED_BODY()

public:
	UCurrentSaveGameViewModel();

	UPROPERTY(FieldNotify, BlueprintReadOnly, Category = "Weekend Utils|Save Game")
	bool bCanContinue = false;

	UFUNCTION(BlueprintCallable, Category = "Weekend Utils|Save Game")
	virtual void BeginUsage();

	UFUNCTION(BlueprintCallable, Category = "Weekend Utils|Save Game")
	virtual void EndUsage();

	UFUNCTION(BlueprintCallable, Category = "Weekend Utils|Save Game")
	virtual void ContinueSaveGame();

	UFUNCTION(BlueprintCallable, Category = "Weekend Utils|Save Game")
	virtual void CreateNewGame();

	// - UObject
	virtual void BeginDestroy() override;
	// --

protected:
	virtual void UpdateForCurrentSaveGame(const FCurrentSaveGame& CurrentSaveGame);
};