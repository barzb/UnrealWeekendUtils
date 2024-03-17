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

#include "SaveGameList_VM.generated.h"

class USaveGamePreset;
class USaveLoadBehavior;
class USaveGameService;
class USaveGameSlot_VM;

///////////////////////////////////////////////////////////////////////////////////////

/**
 * Base class of a ViewModel that lists available SaveGame slots.
 * Works in accord with @USaveGameSlot_VM.
 */
UCLASS(Abstract)
class WEEKENDUTILS_API USaveGameList_VM : public UMVVMViewModelBase,
										  public FGameServiceUser
{
	GENERATED_BODY()

public:
	using FSlotName = FString;

	UPROPERTY(FieldNotify, BlueprintReadOnly)
	TArray<TObjectPtr<USaveGameSlot_VM>> Slots = {};

	USaveGameList_VM();

	UFUNCTION(BlueprintCallable)
	virtual void BeginUsage(TSubclassOf<USaveGameSlot_VM> SlotClass);

	UFUNCTION(BlueprintCallable)
	virtual void EndUsage();

protected:
	UPROPERTY()
	TSubclassOf<USaveGameSlot_VM> SlotViewModelClass = nullptr;
	TWeakObjectPtr<USaveGameService> SaveGameService = nullptr;

	// - UObject
	virtual void BeginDestroy() override;
	// --

	virtual void Update();
	virtual TArray<FSlotName> GatherRelevantSlotNames() PURE_VIRTUAL(GatherRelevantSlotNames, return {};);
	virtual bool AllowsSavingFromWidget() const { return false; }
	virtual bool AllowsLoadingFromWidget() const { return false; }
	virtual bool HandleSaveRequestBySlot(const FSlotName& SlotName) { return false; }
	virtual bool HandleLoadRequestBySlot(const FSlotName& SlotName) { return false; }
};

///////////////////////////////////////////////////////////////////////////////////////

/** ViewModel implementation that lists SaveGame slots available for saving. */
UCLASS()
class WEEKENDUTILS_API USaveGameSaveList_VM : public USaveGameList_VM
{
	GENERATED_BODY()

protected:
	// - USaveGameList_VM
	virtual TArray<FSlotName> GatherRelevantSlotNames() override;
	virtual bool AllowsSavingFromWidget() const override { return true; }
	virtual bool HandleSaveRequestBySlot(const FSlotName& SlotName) override;
	// --
};

///////////////////////////////////////////////////////////////////////////////////////

/** ViewModel implementation that lists SaveGame slots available for loading. */
UCLASS()
class WEEKENDUTILS_API USaveGameLoadList_VM : public USaveGameList_VM
{
	GENERATED_BODY()

protected:
	// - USaveGameList_VM
	virtual TArray<FSlotName> GatherRelevantSlotNames() override;
	virtual bool AllowsLoadingFromWidget() const override { return true; }
	virtual bool HandleLoadRequestBySlot(const FSlotName& SlotName) override;
	// --
};

///////////////////////////////////////////////////////////////////////////////////////

/** ViewModel implementation that lists SaveGame presets available for loading. */
UCLASS()
class WEEKENDUTILS_API USaveGamePresetList_VM : public USaveGameList_VM
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient)
	TArray<TObjectPtr<const USaveGamePreset>> AvailablePresets = {};

	// - USaveGameList_VM
	virtual void Update() override;
	virtual TArray<FSlotName> GatherRelevantSlotNames() override;
	virtual bool AllowsLoadingFromWidget() const override { return true; }
	virtual bool HandleLoadRequestBySlot(const FSlotName& SlotName) override;
	// --
};
