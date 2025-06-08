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
#include "MockSaveGameSerializer.h"
#include "SaveGame/SaveLoadBehavior.h"

#include "MockableSaveLoadBehavior.generated.h"

class USaveGame;
class USaveGameService;
struct FCurrentSaveGame;

/**
 * Behaves like the normal @USaveLoadBehavior but provides events that can be implemented
 * externally to override default behavior. Intended for automation tests only!
 */
UCLASS(ClassGroup = "Tests")
class WEEKENDUTILS_API UMockableSaveLoadBehavior : public USaveLoadBehavior
{
	GENERATED_BODY()

public:
	bool bIsAutosavingAllowed = false;
	FString AutosaveSlotName = "Test_Autosave";
	TSet<FString> SavableSlotNames = { "Test_Slot1", "Test_Slot2" };
	TSet<FString> LoadableSlotNames = { "Test_Slot1", "Test_Slot2", AutosaveSlotName };
	TSubclassOf<USaveGameSerializer> SaveGameSerializerClass = UMockSaveGameSerializer::StaticClass();

	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInitializeCalled, UMockableSaveLoadBehavior&, USaveGameService&)
	static inline FOnInitializeCalled OnNextBehaviourInitializes;

	DECLARE_DELEGATE_OneParam(FOnHandleGameStartCalled, USaveGameService&)
	FOnHandleGameStartCalled OnHandleGameStartCalled;

	DECLARE_DELEGATE_OneParam(FOnHandleGameExitCalled, USaveGameService&)
	FOnHandleGameExitCalled OnHandleGameExitCalled;

	DECLARE_DELEGATE_TwoParams(FOnHandleBeforeSavedCalled, const FCurrentSaveGame& , USaveGameService*)
	FOnHandleBeforeSavedCalled OnHandleBeforeSavedCalled;

	DECLARE_DELEGATE_TwoParams(FOnHandleAfterLoadedCalled, const FCurrentSaveGame& , USaveGameService*)
	FOnHandleAfterLoadedCalled OnHandleAfterLoadedCalled;

	DECLARE_DELEGATE_RetVal_OneParam(USaveGame&, FOnCreateNewSavegameObjectCalled, USaveGameService&)
	FOnCreateNewSavegameObjectCalled OnCreateNewSavegameObjectCalled;

	// - USaveGameBehavior
	virtual void Initialize(USaveGameService& SaveGameService) override
	{
#if WITH_AUTOMATION_TESTS
		OnNextBehaviourInitializes.Broadcast(*this, SaveGameService);
		OnNextBehaviourInitializes.Clear();
#else
		UE_LOG(LogSaveLoadBehavior, Fatal, TEXT("UMockSaveLoadBehavior was initialized outside of a valid automation test environment!"));
#endif
	}

	virtual void HandleGameStart(USaveGameService& SaveGameService) override
	{
		OnHandleGameStartCalled.ExecuteIfBound(SaveGameService);
	}

	virtual void HandleGameExit(USaveGameService& SaveGameService) override
	{
		OnHandleGameExitCalled.ExecuteIfBound(SaveGameService);
	}

	virtual void HandleBeforeSaveGameSaved(const FCurrentSaveGame& SaveGame, USaveGameService* SaveGameService) override
	{
		OnHandleBeforeSavedCalled.ExecuteIfBound(SaveGame, SaveGameService);
	}

	virtual void HandleAfterSaveGameRestored(const FCurrentSaveGame& SaveGame, USaveGameService* SaveGameService) override
	{
		OnHandleAfterLoadedCalled.ExecuteIfBound(SaveGame, SaveGameService);
	}

	virtual TSet<FSlotName> GetSaveSlotNamesAllowedForSaving() const override { return SavableSlotNames; }
	virtual TSet<FSlotName> GetSaveSlotNamesAllowedForLoading() const override { return LoadableSlotNames; }
	virtual FSlotName GetAutosaveSlotName() const override { return AutosaveSlotName; }

	virtual USaveGame& CreateNewSavegameObject(USaveGameService& SaveGameService) const override
	{
		return OnCreateNewSavegameObjectCalled.IsBound()
			 ? OnCreateNewSavegameObjectCalled.Execute(SaveGameService)
			 : Super::CreateNewSavegameObject(SaveGameService);
	}

	virtual TSubclassOf<USaveGameSerializer> GetSaveGameSerializerClass() const override { return SaveGameSerializerClass; }
	// --
};
