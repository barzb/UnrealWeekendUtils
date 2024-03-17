///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "SaveGame/SaveGameService.h"

#include "GameFramework/SaveGame.h"
#include "SaveGame/SaveGameSerializer.h"
#include "SaveGame/SaveGameUtils.h"
#include "SaveGame/SaveLoadBehavior.h"
#include "SaveGame/Settings/SaveGameServiceSettings.h"

DEFINE_LOG_CATEGORY(LogSaveGameService);

namespace
{
	FString LexToString(const USaveGameService::EStatus& Status)
	{
		switch (Status)
		{
			case USaveGameService::EStatus::Uninitialized: return "Uninitialized";
			case USaveGameService::EStatus::Saving: return "Saving";
			case USaveGameService::EStatus::Loading: return "Loading";
			case USaveGameService::EStatus::Idle: return "Idle";
			default: return "???";
		}
	}
}

FAsyncSaveGameHandle USaveGameService::RequestAutosave()
{
	if (!IsAutosavingAllowed())
		return FAsyncSaveGameHandle();

	return RequestSaveCurrentSaveGameToSlot(GetAutosaveSlotName());
}

FAsyncSaveGameHandle USaveGameService::RequestAutosave(const FOnSaveLoadCompleted& Callback)
{
	if (!IsAutosavingAllowed())
		return FAsyncSaveGameHandle();

	return RequestSaveCurrentSaveGameToSlot(GetAutosaveSlotName(), Callback);
}

FAsyncSaveGameHandle USaveGameService::RequestSaveCurrentSaveGameToSlot(const FSlotName& SlotName)
{
	return EnqueueSaveRequest(SlotName, MakeShared<FSaveCurrentSaveGameRequest>(*this));
}

FAsyncSaveGameHandle USaveGameService::RequestSaveCurrentSaveGameToSlot(const FSlotName& SlotName, const FOnSaveLoadCompleted& Callback)
{
	return EnqueueSaveRequest(SlotName, MakeShared<FSaveCurrentSaveGameRequest>(*this, Callback));
}

FAsyncLoadGameHandle USaveGameService::RequestLoadCurrentSaveGameFromSlot(const FSlotName& SlotName)
{
	return EnqueueLoadRequest(SlotName, MakeShared<FLoadToCurrentSaveGameRequest>(*this));
}

FAsyncLoadGameHandle USaveGameService::RequestLoadCurrentSaveGameFromSlot(const FSlotName& SlotName, const FOnSaveLoadCompleted& Callback)
{
	return EnqueueLoadRequest(SlotName, MakeShared<FLoadToCurrentSaveGameRequest>(*this, Callback));
}

FAsyncLoadGameHandle USaveGameService::RequestLoadAndTravelIntoCurrentSaveGameFromSlot(const FSlotName& SlotName)
{
	return EnqueueLoadRequest(SlotName, MakeShared<FLoadAndTravelIntoToCurrentSaveGameRequest>(*this));
}

FAsyncLoadGameHandle USaveGameService::RequestLoadAndTravelIntoCurrentSaveGameFromSlot(const FSlotName& SlotName, const FOnSaveLoadCompleted& Callback)
{
	return EnqueueLoadRequest(SlotName, MakeShared<FLoadAndTravelIntoToCurrentSaveGameRequest>(*this, Callback));
}

bool USaveGameService::IsSaveRequestAlive(const FAsyncSaveGameHandle& Handle) const
{
	if (!Handle.IsValid())
		return false;

	for (auto Itr = PendingSaveRequestsBySlot.CreateConstIterator(); Itr; ++Itr)
	{
		for (const TSharedRef<ISaveLoadRequest>& SaveRequest : Itr.Value())
		{
			if (SaveRequest->Handle == Handle)
				return true;
		}
	}

	for (const TSharedRef<ISaveLoadRequest>& SaveRequest : SaveRequestsInProgress)
	{
		if (SaveRequest->Handle == Handle)
			return true;
	}

	return false;
}

bool USaveGameService::IsLoadRequestAlive(const FAsyncLoadGameHandle& Handle) const
{
	if (!Handle.IsValid())
		return false;

	for (auto Itr = PendingLoadRequestsBySlot.CreateConstIterator(); Itr; ++Itr)
	{
		for (const TSharedRef<ISaveLoadRequest>& LoadRequest : Itr.Value())
		{
			if (LoadRequest->Handle == Handle)
				return true;
		}
	}

	for (const TSharedRef<ISaveLoadRequest>& LoadRequest : LoadRequestsInProgress)
	{
		if (LoadRequest->Handle == Handle)
			return true;
	}

	return false;
}

void USaveGameService::CancelSaveRequest(const FAsyncSaveGameHandle& Handle)
{
	for (TTuple<FSlotName, TArray<TSharedRef<ISaveLoadRequest>>>& RequestsBySlotName : PendingSaveRequestsBySlot)
	{
		TSharedPtr<ISaveLoadRequest> FoundRequest = nullptr;
		for (TSharedRef<ISaveLoadRequest> Request : RequestsBySlotName.Value)
		{
			if (Request->Handle == Handle)
			{
				FoundRequest = Request.ToSharedPtr();
			}
		}

		if (!FoundRequest.IsValid())
			continue;

		FoundRequest->Cancel();
		RequestsBySlotName.Value.Remove(FoundRequest.ToSharedRef());
		if (RequestsBySlotName.Value.IsEmpty())
		{
			PendingSaveRequestsBySlot.Remove(RequestsBySlotName.Key);
		}
		return;
	}
}

void USaveGameService::CancelLoadRequest(const FAsyncLoadGameHandle& Handle)
{
	for (TTuple<FSlotName, TArray<TSharedRef<ISaveLoadRequest>>>& RequestsBySlotName : PendingLoadRequestsBySlot)
	{
		TSharedPtr<ISaveLoadRequest> FoundRequest = nullptr;
		for (TSharedRef<ISaveLoadRequest> Request : RequestsBySlotName.Value)
		{
			if (Request->Handle == Handle)
			{
				FoundRequest = Request.ToSharedPtr();
			}
		}

		if (!FoundRequest.IsValid())
			continue;

		FoundRequest->Cancel();
		RequestsBySlotName.Value.Remove(FoundRequest.ToSharedRef());
		if (RequestsBySlotName.Value.IsEmpty())
		{
			PendingLoadRequestsBySlot.Remove(RequestsBySlotName.Key);
		}
		return;
	}
}

bool USaveGameService::TryLoadCurrentSaveGameFromSlotSynchronous(const FSlotName& SlotName)
{
	if (CachedSaveGames.Contains(SlotName))
		return true;

	if (IsBusySavingOrLoading())
		return false;

	if (!DoesSaveFileExist(SlotName))
		return false;

	SetStatus(EStatus::Loading);

	USaveGame* LoadedSaveGame = PerformSyncLoad(SlotName);
	CurrentSaveGame = FCurrentSaveGame::CreateFromLoadedGame(*LoadedSaveGame, SlotName);

	SetStatus(EStatus::Idle);

	OnAfterRestored.Broadcast(CurrentSaveGame);
	return true;
}

TSet<USaveGame*> USaveGameService::PreloadSaveGamesSynchronous(const TSet<FSlotName>& SlotNames)
{
	TSet<USaveGame*> Result = {};
	if (SlotNames.IsEmpty())
		return Result;

	SetStatus(EStatus::Loading);

	CachedSaveGames.Empty();
	for (const FSlotName& SlotName : SlotNames)
	{
		if (!DoesSaveFileExist(SlotName))
			continue;

		if (USaveGame* LoadedSaveGame = PerformSyncLoad(SlotName))
		{
			CachedSaveGames.Add(SlotName, TStrongObjectPtr(LoadedSaveGame));
			Result.Add(LoadedSaveGame);
		}
	}

	SetStatus(EStatus::Idle);
	OnAvailableSaveGamesChanged.Broadcast();

	return Result;
}

void USaveGameService::PreloadSaveGamesAsync(const TSet<FSlotName>& SlotNames, const FOnPreloadCompleted& Callback)
{
	CachedSaveGames.Empty();
	if (SlotNames.IsEmpty())
	{
		Callback.ExecuteIfBound({});
		return;
	}

	class FPreloadRequest : public ISaveLoadRequest
	{
	public:
		explicit FPreloadRequest(USaveGameService& InService, const TSharedRef<int32>& InRemainingSlots, const TSharedRef<TSet<USaveGame*>>& InResult, const FOnPreloadCompleted& InCallback) :
			ISaveLoadRequest(InService), RemainingSlots(InRemainingSlots), Result(InResult), Callback(InCallback) {}
		virtual void Finish(USaveGame* RequestedSaveGame, bool bSuccess) override
		{
			if (bSuccess)
			{
				Result->Add(RequestedSaveGame);
			}
			if (--(*RemainingSlots) <= 0)
			{
				Callback.ExecuteIfBound(*Result);
			}
		}

		TSharedRef<int32> RemainingSlots;
		TSharedRef<TSet<USaveGame*>> Result;
		FOnPreloadCompleted Callback;
	};

	TSharedRef<int32> RemainingSlots = MakeShared<int32>(0);
	TSharedRef<TSet<USaveGame*>> Results = MakeShared<TSet<USaveGame*>>();

	TMap<FSlotName, TSharedRef<FPreloadRequest>> Requests = {};
	for (const FSlotName& SlotName : SlotNames)
	{
		if (!DoesSaveFileExist(SlotName))
			continue;

		(*RemainingSlots)++;
		Requests.Add(SlotName, MakeShared<FPreloadRequest>(*this, RemainingSlots, Results, Callback));
	}

	for (auto SlotAndRequest = Requests.CreateIterator(); SlotAndRequest; ++SlotAndRequest)
	{
		EnqueueLoadRequest(SlotAndRequest.Key(), SlotAndRequest.Value());
	}
}

void USaveGameService::RestoreAsCurrentSaveGame(USaveGame& SaveGame, TOptional<FSlotName> LoadedFromSlotName)
{
	if (!LoadedFromSlotName.IsSet())
	{
		LoadedFromSlotName = FindSlotName(SaveGame);
	}

	CurrentSaveGame = FCurrentSaveGame::CreateFromLoadedGame(SaveGame, LoadedFromSlotName);
	OnAfterRestored.Broadcast(CurrentSaveGame);
}

void USaveGameService::RestoreAsAndTravelIntoCurrentSaveGame(USaveGame& SaveGame, TOptional<FSlotName> LoadedFromSlotName)
{
	if (CurrentSaveGame != SaveGame)
	{
		RestoreAsCurrentSaveGame(SaveGame, LoadedFromSlotName);
	}

	TryTravelIntoCurrentSaveGame();
}

bool USaveGameService::TryTravelIntoCurrentSaveGame()
{
	check(SaveLoadBehavior);
	return SaveLoadBehavior->TryTravelToSavedLevel(CurrentSaveGame);
}

void USaveGameService::CreateAndRestoreNewSaveGameAsCurrent()
{
	USaveGame& SaveGameObject = SaveLoadBehavior->CreateNewSavegameObject(*this);
	CurrentSaveGame = FCurrentSaveGame::CreateFromNewGame(SaveGameObject);
	OnAfterRestored.Broadcast(CurrentSaveGame);
}

void USaveGameService::ProcessPendingRequests()
{
	if (IsBusySavingOrLoading())
		return;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// Remark: Architecturally, it would be nicer if there was just one queue of pending requests,
	/// since they were build polymorphic, they could perform the save/load in their Process() method.
	/// Additionally, there is currently the potential issue of multiple similar requests for the same
	/// SlotName being grouped together, which can cause multiple "Finish" executions at the same time.
	/// It might be better to accumulate similar requests in one request object to avoid this.
	///////////////////////////////////////////////////////////////////////////////////////////////////

	if (IsSavingAllowed() && PendingSaveRequestsBySlot.Num() > 0)
	{
		auto RequestToProcess = PendingSaveRequestsBySlot.CreateIterator();
		const FSlotName SlotName = RequestToProcess.Key();
		SaveRequestsInProgress += RequestToProcess.Value();
		for (TSharedRef<ISaveLoadRequest> Request : RequestToProcess.Value())
		{
			Request->Process();
		}
		RequestToProcess.RemoveCurrent();

		PerformAsyncSave(SlotName);
		return;
	}

	if (IsLoadingAllowed() && PendingLoadRequestsBySlot.Num() > 0)
	{
		auto RequestToProcess = PendingLoadRequestsBySlot.CreateIterator();
		const FSlotName SlotName = RequestToProcess.Key();
		LoadRequestsInProgress += RequestToProcess.Value();
		for (TSharedRef<ISaveLoadRequest> Request : RequestToProcess.Value())
		{
			Request->Process();
		}
		RequestToProcess.RemoveCurrent();

		PerformAsyncLoad(SlotName);
		return;
	}
}

FSaveLoadLockHandle USaveGameService::LockAutosaving(const UObject& KeyHolder)
{
	const FSaveLoadLockHandle NewKey = FSaveLoadLockHandle::NewGuid();
	FSaveLoadLock& NewLock = ActiveAutosaveLocks.Add(NewKey);
	NewLock.KeyHolder = MakeWeakObjectPtr(&KeyHolder);
	return NewKey;
}

void USaveGameService::UnlockAutosaving(const FSaveLoadLockHandle& Key)
{
	if (!ActiveAutosaveLocks.Contains(Key))
		return;

	ActiveAutosaveLocks.Remove(Key);
	ProcessPendingRequests();
}

FSaveLoadLockHandle USaveGameService::LockSaving(const UObject& KeyHolder)
{
	const FSaveLoadLockHandle NewKey = FSaveLoadLockHandle::NewGuid();
	FSaveLoadLock& NewLock = ActiveSaveLocks.Add(NewKey);
	NewLock.KeyHolder = MakeWeakObjectPtr(&KeyHolder);
	return NewKey;
}

void USaveGameService::UnlockSaving(const FSaveLoadLockHandle& Key)
{
	if (!ActiveSaveLocks.Contains(Key))
		return;

	ActiveSaveLocks.Remove(Key);
	ProcessPendingRequests();
}

FSaveLoadLockHandle USaveGameService::LockLoading(const UObject& KeyHolder)
{
	const FSaveLoadLockHandle NewKey = FSaveLoadLockHandle::NewGuid();
	FSaveLoadLock& NewLock = ActiveLoadLocks.Add(NewKey);
	NewLock.KeyHolder = MakeWeakObjectPtr(&KeyHolder);
	return NewKey;
}

void USaveGameService::UnlockLoading(const FSaveLoadLockHandle& Key)
{
	if (!ActiveLoadLocks.Contains(Key))
		return;

	ActiveLoadLocks.Remove(Key);
	ProcessPendingRequests();
}

bool USaveGameService::IsAutosavingAllowed() const
{
	return (IsSavingAllowed() && ActiveAutosaveLocks.IsEmpty());
}

bool USaveGameService::IsSavingAllowed() const
{
	return (!IsBusySavingOrLoading() && ActiveSaveLocks.IsEmpty());
}

bool USaveGameService::IsLoadingAllowed() const
{
	return (!IsBusySavingOrLoading() && ActiveLoadLocks.IsEmpty());
}

bool USaveGameService::IsBusyLoading() const
{
	return (LoadRequestsInProgress.Num() > 0);
}

bool USaveGameService::IsBusySaving() const
{
	return (SaveRequestsInProgress.Num() > 0);
}

bool USaveGameService::IsBusySavingOrLoading() const
{
	return (IsBusyLoading() || IsBusySaving());
}

USaveLoadBehavior& USaveGameService::CreateSaveLoadBehavior()
{
	checkf(!SaveLoadBehavior, TEXT("SaveLoadBehavior was already created"));

	const USaveGameServiceSettings& Settings = *GetDefault<USaveGameServiceSettings>();
	TSoftClassPtr<USaveLoadBehavior> BehaviorClass = Settings.SaveLoadBehavior;

#if WITH_EDITORONLY_DATA
	BehaviorClass = (GetWorld()->WorldType == EWorldType::PIE)
		? Settings.PlayInEditorSaveLoadBehavior
		: Settings.PlayInStandaloneSaveLoadBehavior;
#endif

#if WITH_AUTOMATION_TESTS
	if (GIsAutomationTesting)
	{
		BehaviorClass = Settings.AutomationTestSaveLoadBehavior;
	}
#endif

	USaveLoadBehavior* Behavior = NewObject<USaveLoadBehavior>(this, BehaviorClass.LoadSynchronous());
	check(Behavior);

	UE_LOG(LogSaveGameService, Log, TEXT("%s created %s"), *GetName(), *Behavior->GetName());
	OnBeforeSaved.AddUObject(Behavior, &USaveLoadBehavior::HandleBeforeSaveGameSaved, this);
	OnAfterRestored.AddUObject(Behavior, &USaveLoadBehavior::HandleAfterSaveGameRestored, this);
	Behavior->Initialize(*this);

	return *Behavior;
}

USaveGameSerializer& USaveGameService::CreateSaveGameSerializer()
{
	checkf(!SaveGameSerializer, TEXT("SaveGameSerializer was already created"));
	checkf(SaveLoadBehavior, TEXT("SaveLoadBehavior must be created before SaveGameSerializer"));

	const TSubclassOf<USaveGameSerializer> SerializerClass = SaveLoadBehavior->GetSaveGameSerializerClass();
	check(SerializerClass);

	USaveGameSerializer* Serializer = NewObject<USaveGameSerializer>(this, SerializerClass);
	check(Serializer);
	UE_LOG(LogSaveGameService, Log, TEXT("%s created %s"), *GetName(), *Serializer->GetName());

	return *Serializer;
}

void USaveGameService::StartService()
{
	SaveLoadBehavior = &CreateSaveLoadBehavior();
	SaveGameSerializer = &CreateSaveGameSerializer();

	SetStatus(EStatus::Idle);

	SaveLoadBehavior->HandleGameStart(*this);

	// Need something to save to -> create a new SG object:
	if (!CurrentSaveGame.IsValid())
	{
		CreateAndRestoreNewSaveGameAsCurrent();
	}

	FWorldDelegates::OnPostWorldInitialization.AddWeakLambda(this, [this](UWorld* World, const UWorld::InitializationValues)
	{
		HandleLevelChanged(World);
	});
}

void USaveGameService::ShutdownService()
{
	SetStatus(EStatus::Uninitialized);
	SaveGameSerializer = nullptr;

	if (SaveLoadBehavior)
	{
		OnBeforeSaved.RemoveAll(SaveLoadBehavior);
		OnAfterRestored.RemoveAll(SaveLoadBehavior);
		SaveLoadBehavior->HandleGameExit(*this);
		SaveLoadBehavior = nullptr;
	}

	CurrentSaveGame.Reset();
	CachedSaveGames.Empty();

	PendingSaveRequestsBySlot.Empty();
	PendingLoadRequestsBySlot.Empty();

	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
}

FAsyncSaveGameHandle USaveGameService::EnqueueSaveRequest(const FSlotName& SlotName, const TSharedRef<ISaveLoadRequest>& Request)
{
	if (!IsSavingAllowed())
	{
		Request->Finish(nullptr, false);
		return FAsyncSaveGameHandle();
	}

	PendingSaveRequestsBySlot.FindOrAdd(SlotName).Emplace(Request);
	ProcessPendingRequests();
	return Request->Handle;
}

void USaveGameService::ConsumeSaveRequestsInProgress(USaveGame* SavedSaveGame, bool bSuccess)
{
	for (const TSharedRef<ISaveLoadRequest>& SaveRequest : SaveRequestsInProgress)
	{
		SaveRequest->Finish(SavedSaveGame, bSuccess);
	}

	SaveRequestsInProgress.Empty();
}

FAsyncLoadGameHandle USaveGameService::EnqueueLoadRequest(const FSlotName& SlotName, const TSharedRef<ISaveLoadRequest>& Request)
{
	if (!IsLoadingAllowed())
	{
		Request->Finish(nullptr, false);
		return FAsyncLoadGameHandle();
	}

	if (CachedSaveGames.Contains(SlotName))
	{
		Request->Finish(CachedSaveGames[SlotName].Get(), true);
	}
	else
	{
		PendingLoadRequestsBySlot.FindOrAdd(SlotName).Emplace(Request);
		ProcessPendingRequests();
	}

	return Request->Handle;
}

void USaveGameService::ConsumeLoadRequestsInProgress(USaveGame* LoadedSaveGame, bool bSuccess)
{
	for (const TSharedRef<ISaveLoadRequest>& LoadRequest : LoadRequestsInProgress)
	{
		LoadRequest->Finish(LoadedSaveGame, bSuccess);
	}

	LoadRequestsInProgress.Empty();
}

USaveGameService::FSlotName USaveGameService::GetAutosaveSlotName() const
{
	ensure(SaveLoadBehavior);
	return (SaveLoadBehavior ? SaveLoadBehavior->GetAutosaveSlotName() : FSlotName());
}

TOptional<USaveGameService::FSlotName> USaveGameService::GetMostRecentlySavedSlotName() const
{
	return CurrentSaveGame.GetSlotLastSavedTo();
}

TOptional<USaveGameService::FSlotName> USaveGameService::FindSlotName(const USaveGame& SaveGame) const
{
	const FSlotName* FoundSlotName = CachedSaveGames.FindKey(TStrongObjectPtr(const_cast<USaveGame*>(&SaveGame)));
	return (FoundSlotName ? *FoundSlotName : TOptional<FSlotName>());
}

const USaveGame* USaveGameService::GetCachedSaveGameAtSlot(const FSlotName& SlotName) const
{
	if (!CachedSaveGames.Contains(SlotName))
		return nullptr;

	return CachedSaveGames[SlotName].Get();
}

bool USaveGameService::DoesSaveFileExist(const FSlotName& SlotName) const
{
	return (SaveGameSerializer && SaveGameSerializer->DoesSaveGameExist(SlotName, GetCurrentUserIndex()));
}

TSet<USaveGameService::FSlotName> USaveGameService::GetSlotNamesAllowedForSaving() const
{
	if (!IsSavingAllowed())
		return {};

	return SaveLoadBehavior->GetSaveSlotNamesAllowedForSaving();
}

TSet<USaveGameService::FSlotName> USaveGameService::GetSlotNamesAllowedForLoading() const
{
	if (!IsLoadingAllowed())
		return {};

	TSet<FSlotName> Result = {};
	for (const FSlotName& SlotName : SaveLoadBehavior->GetSaveSlotNamesAllowedForLoading())
	{
		if (DoesSaveFileExist(SlotName))
		{
			Result.Add(SlotName);
		}
	}

	return Result;
}

void USaveGameService::PerformAsyncSave(const FSlotName& SlotName)
{
	const int32& UserIndex = GetCurrentUserIndex();
	if (!CurrentSaveGame.IsValid())
	{
		HandleAsyncSaveCompleted(SlotName, UserIndex, false);
		return;
	}

	SetStatus(EStatus::Saving);

	// Last chance to populate the save game with data:
	OnBeforeSaved.Broadcast(CurrentSaveGame);

	CachedSaveGames.Add(SlotName, TStrongObjectPtr(CurrentSaveGame.GetMutablePtr()));
	SaveGameSerializer->AsyncSaveGameToSlot(CurrentSaveGame.GetRef(), SlotName, UserIndex,
		USaveGameSerializer::FOnAsyncSaveCompleted::CreateUObject(this, &ThisClass::HandleAsyncSaveCompleted));
}

void USaveGameService::HandleAsyncSaveCompleted(const FSlotName& SlotName, const int32 UserIndex, bool bSuccess)
{
	CurrentSaveGame.UpdateTimeOfLastSave();
	CurrentSaveGame.SetSlotLastSavedTo(SlotName);

	ConsumeSaveRequestsInProgress(CurrentSaveGame.GetMutablePtr(), bSuccess);

	SetStatus(EStatus::Idle);
	ProcessPendingRequests();
}

void USaveGameService::PerformAsyncLoad(const FSlotName& SlotName)
{
	if (!DoesSaveFileExist(SlotName))
		return;

	SetStatus(EStatus::Loading);

	const int32& UserIndex = GetCurrentUserIndex();
	SaveGameSerializer->AsyncLoadGameFromSlot(SlotName, UserIndex,
		USaveGameSerializer::FOnAsyncLoadCompleted::CreateUObject(this, &ThisClass::HandleAsyncLoadCompleted));
}

USaveGame* USaveGameService::PerformSyncLoad(const FSlotName& SlotName)
{
	const int32 UserIndex = GetCurrentUserIndex();

	USaveGame* LoadedSaveGame = nullptr;
	SaveGameSerializer->TryLoadGameFromSlot(SlotName, UserIndex, OUT LoadedSaveGame);
	if (LoadedSaveGame && !CachedSaveGames.Contains(SlotName))
	{
		CachedSaveGames.Add(SlotName, TStrongObjectPtr(LoadedSaveGame));
		OnAvailableSaveGamesChanged.Broadcast();
	}

	return LoadedSaveGame;
}

void USaveGameService::HandleAsyncLoadCompleted(const FSlotName& SlotName, const int32 UserIndex, USaveGame* LoadedSaveGame)
{
	if (LoadedSaveGame && !CachedSaveGames.Contains(SlotName))
	{
		CachedSaveGames.Add(SlotName, TStrongObjectPtr(LoadedSaveGame));
		OnAvailableSaveGamesChanged.Broadcast();
	}

	ConsumeLoadRequestsInProgress(LoadedSaveGame, IsValid(LoadedSaveGame));

	SetStatus(EStatus::Idle);
	ProcessPendingRequests();
}

void USaveGameService::HandleLevelChanged(UWorld* NewWorld)
{
	UpdateSaveLockForLevel(NewWorld);

	SaveLoadBehavior->HandleLevelChanged(*this, NewWorld);
}

void USaveGameService::UpdateSaveLockForLevel(UWorld* NewWorld)
{
	const bool bIsSavingAllowed = USaveGameUtils::IsSavingAllowedForWorld(NewWorld);
	const bool bWasSavingAllowed = !CurrentLevelSaveLock.IsSet();
	if (bIsSavingAllowed == bWasSavingAllowed)
		return;

	if (bIsSavingAllowed)
	{
		UnlockSaving(*CurrentLevelSaveLock);
	}
	else
	{
		CurrentLevelSaveLock = LockSaving(*NewWorld);
	}
}

void USaveGameService::SetStatus(const EStatus& NewStatus)
{
	if (CurrentStatus == NewStatus)
		return;

	UE_LOG(LogSaveGameService, Verbose, TEXT("%s changed status: %s -> %s"), *GetName(), *LexToString(CurrentStatus), *LexToString(NewStatus));
	CurrentStatus = NewStatus;
	OnStatusChanged.Broadcast(NewStatus);
}

///////////////////////////////////////////////////////////////////////////////////////
/// REQUESTS

void USaveGameService::ISaveLoadRequest::Finish(USaveGame* RequestedSaveGame, bool bSuccess)
{
	if (RequestCallback.IsSet())
	{
		RequestCallback->ExecuteIfBound(RequestedSaveGame, bSuccess);
	}
}

void USaveGameService::FLoadToCurrentSaveGameRequest::Finish(USaveGame* RequestedSaveGame, bool bSuccess)
{
	if (bSuccess)
	{
		Service.RestoreAsCurrentSaveGame(*RequestedSaveGame);
	}

	ISaveLoadRequest::Finish(RequestedSaveGame, bSuccess);
}

void USaveGameService::FLoadAndTravelIntoToCurrentSaveGameRequest::Finish(USaveGame* RequestedSaveGame, bool bSuccess)
{
	if (bSuccess)
	{
		Service.RestoreAsAndTravelIntoCurrentSaveGame(*RequestedSaveGame);
	}

	ISaveLoadRequest::Finish(RequestedSaveGame, bSuccess);
}
