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

#include "CurrentSaveGame_VM.generated.h"

struct FCurrentSaveGame;

/**
 * ViewModel that represents the currently loaded SaveGame slot.
 */
UCLASS()
class WEEKENDUTILS_API UCurrentSaveGame_VM : public UMVVMViewModelBase,
											 public FGameServiceUser
{
	GENERATED_BODY()

public:
	UCurrentSaveGame_VM();

	UPROPERTY(FieldNotify, BlueprintReadOnly)
	bool bCanContinue = false;

	UFUNCTION(BlueprintCallable)
	virtual void BeginUsage();

	UFUNCTION(BlueprintCallable)
	virtual void EndUsage();

	UFUNCTION(BlueprintCallable)
	virtual void ContinueSaveGame();

	UFUNCTION(BlueprintCallable)
	virtual void CreateNewGame();

	// - UObject
	virtual void BeginDestroy() override;
	// --

protected:
	virtual void UpdateForCurrentSaveGame(const FCurrentSaveGame& CurrentSaveGame);
};
