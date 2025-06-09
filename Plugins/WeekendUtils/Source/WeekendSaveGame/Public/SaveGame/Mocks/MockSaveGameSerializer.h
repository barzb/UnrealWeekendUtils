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
#include "SaveGame/SaveGameSerializer.h"

#include "MockSaveGameSerializer.generated.h"

class USaveGame;

/**
 * Uses the same serialization as the parent class but instead of saving from or loading to disk,
 * this mock keeps the serialized data in transient memory. Thus, all async calls become synchronous.
 * (i) The lifetime of the serialized data is bound to the lifetime of the serializer.
 */
UCLASS(ClassGroup = "Tests")
class WEEKENDSAVEGAME_API UMockSaveGameSerializer : public USaveGameSerializer
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	mutable TArray<TObjectPtr<USaveGame>> SerializedSaveGameObjects = {};
	TMap<FSlotName, TArray<uint8>> PretendedSaveGamesOnDisk = {};

	// - USaveGameSerializer
	using USaveGameSerializer::TrySaveGameToSlot;
	using USaveGameSerializer::TryLoadGameFromSlot;
	virtual bool TrySerializeSaveGame(USaveGame& InSaveGameObject, TArray<uint8>& OutSaveData) const override;
	virtual bool TryDeserializeSaveGame(const TArray<uint8>& InSaveData, USaveGame*& OutSaveGameObject) const override;
	virtual bool DoesSaveGameExist(const FSlotName& SlotName, const int32 UserIndex) const override;
	virtual bool TrySaveDataToSlot(const TArray<uint8>& InSaveData, const FSlotName& SlotName, const int32 UserIndex) override;
	virtual void AsyncSaveGameToSlot(USaveGame& SaveGameObject, const FSlotName& SlotName, const int32 UserIndex, FOnAsyncSaveCompleted Callback) override;
	virtual bool TryLoadDataFromSlot(const FSlotName& SlotName, const int32 UserIndex, TArray<uint8>& OutSaveData) override;
	virtual void AsyncLoadGameFromSlot(const FSlotName& SlotName, const int32 UserIndex, FOnAsyncLoadCompleted Callback) override;
	virtual bool TryDeleteGameInSlot(const FSlotName& SlotName, const int32 UserIndex) override;
	// --
};
