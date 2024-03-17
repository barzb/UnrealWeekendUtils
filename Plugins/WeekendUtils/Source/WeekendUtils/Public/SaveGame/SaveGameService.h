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
#include "CurrentSaveGame.h"
#include "GameFramework/SaveGame.h"
#include "GameService/GameServiceBase.h"

#include "SaveGameService.generated.h"

class USaveGame;
class USaveGameDataProcessor;
class USaveGameSerializer;
class USaveLoadBehavior;

using FAsyncSaveGameHandle = FGuid;
using FAsyncLoadGameHandle = FGuid;
using FSaveLoadLockHandle = FGuid;

WEEKENDUTILS_API DECLARE_LOG_CATEGORY_EXTERN(LogSaveGameService, Log, All);

DECLARE_DELEGATE_TwoParams(FOnSaveLoadCompleted, USaveGame*, bool /*bSuccess*/)
DECLARE_DELEGATE_OneParam(FOnPreloadCompleted, const TSet<USaveGame*>&)

/**
 * Central API for saving and loading SaveGames.
 * Once configured, the service starts with the first applicable world, but remains alive
 * even when the level is changed (lifetime is bound to the @UGameInstance).
 *
 * This service splits off parts of its implementation as polymorphic subobjects that can
 * be customized by each project (see @USaveLoadBehavior and @USaveGameSerializer).
 *
 * Other game service classes that need to save and restore data from the SaveGame should
 * consider deriving from @URestorableGameService.
 */
UCLASS()
class WEEKENDUTILS_API USaveGameService : public UGameServiceBase
{
	GENERATED_BODY()

public:
	USaveGameService()
	{
		// (i) Service will outlive the world it was created in to ensure SaveGame persistence.
		Lifetime = EGameServiceLifetime::ShutdownWithGameInstance;
	}

	using FSlotName = FString;
	enum class EStatus : uint8
	{
		Uninitialized,
		Saving,
		Loading,
		Idle
	};

	///////////////////////////////////////////////////////////////////////////////////////
	/// MULTICAST EVENTS

	/** Event fired after the (async) status of the service has changed. */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnStatusChanged, EStatus)
	FOnStatusChanged OnStatusChanged;

	/** Event fired after the list of SaveGames available for saving/loading has changed. */
	DECLARE_MULTICAST_DELEGATE(FOnAvailableSaveGamesChanged)
	FOnAvailableSaveGamesChanged OnAvailableSaveGamesChanged;

	/** Event fired right before the current SaveGame is saved. Last chance to push data into the SaveGame. */
	FCurrentSaveGame::FOnBeforeSaved OnBeforeSaved;

	/** Event fired right after the current SaveGame was restored. */
	FCurrentSaveGame::FOnAfterRestored OnAfterRestored;

	///////////////////////////////////////////////////////////////////////////////////////
	/// REQUESTS

	/** Asynchronously save the current state of the game into the Autosave file slot. */
	FAsyncSaveGameHandle RequestAutosave();
	FAsyncSaveGameHandle RequestAutosave(const FOnSaveLoadCompleted& Callback);

	/** Asynchronously save the current state of the game into the given SaveGame file slot. */
	FAsyncSaveGameHandle RequestSaveCurrentSaveGameToSlot(const FSlotName& SlotName);
	FAsyncSaveGameHandle RequestSaveCurrentSaveGameToSlot(const FSlotName& SlotName, const FOnSaveLoadCompleted& Callback);

	/** Asynchronously load and restore a SaveGame file as current SaveGame from given slot. */
	FAsyncLoadGameHandle RequestLoadCurrentSaveGameFromSlot(const FSlotName& SlotName);
	FAsyncLoadGameHandle RequestLoadCurrentSaveGameFromSlot(const FSlotName& SlotName, const FOnSaveLoadCompleted& Callback);

	/**
	 * Asynchronously load and restore a SaveGame file as current SaveGame from given slot.
	 * Afterwards, travel into the level stored in the SaveGame. The CurrentSaveGame will persist even after travelling.
	 */
	FAsyncLoadGameHandle RequestLoadAndTravelIntoCurrentSaveGameFromSlot(const FSlotName& SlotName);
	FAsyncLoadGameHandle RequestLoadAndTravelIntoCurrentSaveGameFromSlot(const FSlotName& SlotName, const FOnSaveLoadCompleted& Callback);

	/** @returns whether a given save request handle is still active (pending or being processed). */
	bool IsSaveRequestAlive(const FAsyncSaveGameHandle& Handle) const;
	/** @returns whether a given load request handle is still active (pending or being processed). */
	bool IsLoadRequestAlive(const FAsyncLoadGameHandle& Handle) const;

	/** Cancel an active save request by handle. Nothing happens if the request does not exist (anymore). */
	void CancelSaveRequest(const FAsyncSaveGameHandle& Handle);
	/** Cancel an active load request by handle. Nothing happens if the request does not exist (anymore). */
	void CancelLoadRequest(const FAsyncLoadGameHandle& Handle);

	/**
	 * Attempt to process any pending async requests, if saving/loading is currently allowed.
	 * This doesn't need to be called from the outside as it is automatically invoked internally.
	 */
	virtual void ProcessPendingRequests();

	/**
	 * Attempts to synchronously load and restore a SaveGame file as current SaveGame from given slot.
	 * @returns whether this operation was allowed and successful.
	 */
	virtual bool TryLoadCurrentSaveGameFromSlotSynchronous(const FSlotName& SlotName);

	/** Synchronously loads (but not restores) SaveGame files into a persistent cache. Preloading is useful for displaying available SaveGames. */
	virtual TSet<USaveGame*> PreloadSaveGamesSynchronous(const TSet<FSlotName>& SlotNames);
	/** Asynchronously loads (but not restores) SaveGame files into a persistent cache. Preloading is useful for displaying available SaveGames. */
	virtual void PreloadSaveGamesAsync(const TSet<FSlotName>& SlotNames, const FOnPreloadCompleted& Callback);

	/** Sets and restores an already loaded SaveGame as current SaveGame. */
	virtual void RestoreAsCurrentSaveGame(USaveGame& SaveGame, TOptional<FSlotName> LoadedFromSlotName = {});
	/** Sets and restores an already loaded SaveGame as current SaveGame. Afterwards, travel into the level stored in the SaveGame. */
	virtual void RestoreAsAndTravelIntoCurrentSaveGame(USaveGame& SaveGame, TOptional<FSlotName> LoadedFromSlotName = {});
	/** Travel into the level that is stored in the current SaveGame. @returns whether this was successful. */
	virtual bool TryTravelIntoCurrentSaveGame();

	/** Resets the current SaveGame to a new one and tells all restoring listeners. */
	virtual void CreateAndRestoreNewSaveGameAsCurrent();

	///////////////////////////////////////////////////////////////////////////////////////
	/// LOCKS

	virtual FSaveLoadLockHandle LockAutosaving(const UObject& KeyHolder);
	virtual void UnlockAutosaving(const FSaveLoadLockHandle& Key);

	virtual FSaveLoadLockHandle LockSaving(const UObject& KeyHolder);
	virtual void UnlockSaving(const FSaveLoadLockHandle& Key);

	virtual FSaveLoadLockHandle LockLoading(const UObject& KeyHolder);
	virtual void UnlockLoading(const FSaveLoadLockHandle& Key);

	///////////////////////////////////////////////////////////////////////////////////////
	/// COMPOSITE SUBOBJECTS

	template <typename T = USaveLoadBehavior>
	T* GetSaveLoadBehavior() const;

	template <typename T = USaveGameSerializer>
	T* GetSaveGameSerializer() const;

	///////////////////////////////////////////////////////////////////////////////////////
	/// INFORMATION

	FORCEINLINE const FCurrentSaveGame& GetCurrentSaveGame() const { return CurrentSaveGame; }
	FORCEINLINE virtual uint32 GetCurrentUserIndex() const { return 0; }
	FORCEINLINE EStatus GetCurrentStatus() const { return CurrentStatus; }

	virtual bool IsAutosavingAllowed() const;
	virtual bool IsSavingAllowed() const;
	virtual bool IsLoadingAllowed() const;

	virtual bool IsBusyLoading() const;
	virtual bool IsBusySaving() const;
	virtual bool IsBusySavingOrLoading() const;

	virtual FSlotName GetAutosaveSlotName() const;
	virtual TOptional<FSlotName> GetMostRecentlySavedSlotName() const;
	virtual TOptional<FSlotName> FindSlotName(const USaveGame& SaveGame) const;
	const USaveGame* GetCachedSaveGameAtSlot(const FSlotName& SlotName) const;
	virtual bool DoesSaveFileExist(const FSlotName& SlotName) const;
	virtual TSet<FSlotName> GetSlotNamesAllowedForSaving() const;
	virtual TSet<FSlotName> GetSlotNamesAllowedForLoading() const;

protected:
	///////////////////////////////////////////////////////////////////////////////////////
	/// STATE

	UPROPERTY()
	TObjectPtr<USaveLoadBehavior> SaveLoadBehavior = nullptr;

	UPROPERTY()
	TObjectPtr<USaveGameSerializer> SaveGameSerializer = nullptr;

	UPROPERTY()
	FCurrentSaveGame CurrentSaveGame;

	TMap<FSlotName, TStrongObjectPtr<USaveGame>> CachedSaveGames = {};

	EStatus CurrentStatus = EStatus::Uninitialized;

	///////////////////////////////////////////////////////////////////////////////////////
	/// REQUESTS

	class ISaveLoadRequest : public TSharedFromThis<ISaveLoadRequest>
	{
	public:
		virtual void Process() {};
		virtual void Finish(USaveGame* RequestedSaveGame, bool bSuccess);
		virtual void Cancel() {}

		USaveGameService& Service;
		FGuid Handle = FGuid::NewGuid();
		TOptional<FOnSaveLoadCompleted> RequestCallback = {};

	protected:
		virtual ~ISaveLoadRequest() = default;
		explicit ISaveLoadRequest(USaveGameService& InService) : Service(InService) {}
		explicit ISaveLoadRequest(USaveGameService& InService, const FOnSaveLoadCompleted& Callback) : Service(InService), RequestCallback(Callback) {}
	};

	class FLoadToCurrentSaveGameRequest : public ISaveLoadRequest
	{
	public:
		explicit FLoadToCurrentSaveGameRequest(USaveGameService& InService) : ISaveLoadRequest(InService) {}
		explicit FLoadToCurrentSaveGameRequest(USaveGameService& InService, const FOnSaveLoadCompleted& Callback) : ISaveLoadRequest(InService, Callback) {}
		virtual void Finish(USaveGame* RequestedSaveGame, bool bSuccess) override;
	};

	class FLoadAndTravelIntoToCurrentSaveGameRequest : public ISaveLoadRequest
	{
	public:
		explicit FLoadAndTravelIntoToCurrentSaveGameRequest(USaveGameService& InService) : ISaveLoadRequest(InService) {}
		explicit FLoadAndTravelIntoToCurrentSaveGameRequest(USaveGameService& InService, const FOnSaveLoadCompleted& Callback) : ISaveLoadRequest(InService, Callback) {}
		virtual void Finish(USaveGame* RequestedSaveGame, bool bSuccess) override;
	};

	class FSaveCurrentSaveGameRequest : public ISaveLoadRequest
	{
	public:
		explicit FSaveCurrentSaveGameRequest(USaveGameService& InService) : ISaveLoadRequest(InService) {}
		explicit FSaveCurrentSaveGameRequest(USaveGameService& InService, const FOnSaveLoadCompleted& Callback) : ISaveLoadRequest(InService, Callback) {}
	};

	TMap<FSlotName, TArray<TSharedRef<ISaveLoadRequest>>> PendingSaveRequestsBySlot = {};
	TArray<TSharedRef<ISaveLoadRequest>> SaveRequestsInProgress = {};

	TMap<FSlotName, TArray<TSharedRef<ISaveLoadRequest>>> PendingLoadRequestsBySlot = {};
	TArray<TSharedRef<ISaveLoadRequest>> LoadRequestsInProgress = {};

	///////////////////////////////////////////////////////////////////////////////////////
	/// LOCKS

	struct FSaveLoadLock
	{
		TWeakObjectPtr<const UObject> KeyHolder = nullptr;
		FString ContextString = FString();
	};

	TMap<FSaveLoadLockHandle, FSaveLoadLock> ActiveAutosaveLocks = {};
	TMap<FSaveLoadLockHandle, FSaveLoadLock> ActiveSaveLocks = {};
	TMap<FSaveLoadLockHandle, FSaveLoadLock> ActiveLoadLocks = {};
	TOptional<FSaveLoadLockHandle> CurrentLevelSaveLock = {};

	///////////////////////////////////////////////////////////////////////////////////////
	/// OVERRIDES

	// - UGameServiceBase
	virtual void StartService() override;
	virtual void ShutdownService() override;
	// --

	///////////////////////////////////////////////////////////////////////////////////////
	/// REQUESTS

	FAsyncSaveGameHandle EnqueueSaveRequest(const FSlotName& SlotName, const TSharedRef<ISaveLoadRequest>& Request);
	void ConsumeSaveRequestsInProgress(USaveGame* SavedSaveGame, bool bSuccess);

	FAsyncLoadGameHandle EnqueueLoadRequest(const FSlotName& SlotName, const TSharedRef<ISaveLoadRequest>& Request);
	void ConsumeLoadRequestsInProgress(USaveGame* LoadedSaveGame, bool bSuccess);

	///////////////////////////////////////////////////////////////////////////////////////
	/// SAVE & LOAD

	virtual USaveLoadBehavior& CreateSaveLoadBehavior();
	virtual USaveGameSerializer& CreateSaveGameSerializer();

	virtual void PerformAsyncSave(const FSlotName& SlotName);
	virtual void PerformAsyncLoad(const FSlotName& SlotName);
	virtual USaveGame* PerformSyncLoad(const FSlotName& SlotName);

	virtual void HandleAsyncSaveCompleted(const FString& SlotName, const int32 UserIndex, bool bSuccess);
	virtual void HandleAsyncLoadCompleted(const FString& SlotName, const int32 UserIndex, USaveGame* LoadedSaveGame);

	///////////////////////////////////////////////////////////////////////////////////////
	/// MISC

	virtual void HandleLevelChanged(UWorld* NewWorld);
	virtual void UpdateSaveLockForLevel(UWorld* NewWorld);

	virtual void SetStatus(const EStatus& NewStatus);
};

///////////////////////////////////////////////////////////////////////////////////////
/// TEMPLATES @USaveGameService

template <typename T>
T* USaveGameService::GetSaveLoadBehavior() const
{
	return Cast<T>(SaveLoadBehavior);
}

template <typename T>
T* USaveGameService::GetSaveGameSerializer() const
{
	return Cast<T>(SaveGameSerializer);
}
