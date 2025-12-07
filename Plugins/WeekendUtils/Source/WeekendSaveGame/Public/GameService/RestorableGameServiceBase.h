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
#include "GameService/AsyncGameServiceBase.h"

#include "RestorableGameServiceBase.generated.h"

class USaveGameService;
struct FCurrentSaveGame;

/**
 * Asynchronous game service base class that listens to restored SaveGames from the @USaveGameService.
 * Offers virtual methods to implement for derived classes that respond to SaveGame events.
 * Dependencies to restorable services should call @WaitUntilServiceIsRunning().
 */
UCLASS(Abstract)
class WEEKENDSAVEGAME_API URestorableGameServiceBase : public UAsyncGameServiceBase
{
	GENERATED_BODY()

public:
	// - UAsyncGameServiceBase
	virtual FGameServiceUserConfig ConfigureGameServiceUser() const override;
	virtual void BeginServiceStart() override final;
	virtual void BeginServiceShutdown(bool bIsWorldTearingDown) override;
	virtual TOptional<FString> GetServiceStatusInfo() const override;
	// --

	/**
	 * Called right before the service has started running. This includes waiting for other async service dependencies.
	 * The passed @FCurrentSaveGame contains either a valid restored SaveGame, or a newly created one.
	 */
	virtual void StartRestorableService(const FCurrentSaveGame& SaveGame) PURE_VIRTUAL(StartRestorableService);

	/** Called every time the current SaveGame is about to be saved, so the service can write data into it. */
	virtual void WriteToSaveGame(const FCurrentSaveGame& InOutSaveGame) {}

	/**
	 * Called after the current SaveGame was loaded. This is not fired if the service is not yet running.
	 * The initial restoration is being forwarded to @StartRestorableService instead.
	 */
	virtual void RestoreFromSaveGame(const FCurrentSaveGame& SaveGame) {}

protected:
	TWeakObjectPtr<USaveGameService> SaveGameService = nullptr;

private:
	bool bIsWaitingForSaveGameRestore = false;

	void HandleSaveGameLoaded(const FCurrentSaveGame& SaveGame);
};
