///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "SaveGame/Mocks/MockSaveGameSerializer.h"

#include "GameFramework/SaveGame.h"

bool UMockSaveGameSerializer::TrySerializeSaveGame(USaveGame& InSaveGameObject, TArray<uint8>& OutSaveData) const
{
	// Store a copy so the "saved" version won't change when the originally passed object mutates after serialization:
	USaveGame* CopyOfSaveGameObject =
		DuplicateObject<USaveGame>(&InSaveGameObject, InSaveGameObject.GetOuter(), FName(InSaveGameObject.GetName() + "_Serialized"));

	// Give out the index of the "serialized" object as only entry of the output byte array:
	const int32 Index = SerializedSaveGameObjects.AddUnique(CopyOfSaveGameObject);
	OutSaveData = {static_cast<uint8>(Index)};
	return (Index != INDEX_NONE);
}

bool UMockSaveGameSerializer::TryDeserializeSaveGame(const TArray<uint8>& InSaveData, USaveGame*& OutSaveGameObject) const
{
	const int32 Index = (InSaveData.IsValidIndex(0) ? static_cast<int32>(InSaveData[0]) : INDEX_NONE);
	OutSaveGameObject = (SerializedSaveGameObjects.IsValidIndex(Index) ? SerializedSaveGameObjects[Index] : nullptr);
	return (OutSaveGameObject != nullptr);
}

bool UMockSaveGameSerializer::DoesSaveGameExist(const FSlotName& SlotName, const int32 UserIndex) const
{
	return PretendedSaveGamesOnDisk.Contains(SlotName);
}

bool UMockSaveGameSerializer::TrySaveDataToSlot(const TArray<uint8>& InSaveData, const FSlotName& SlotName, const int32 UserIndex)
{
	PretendedSaveGamesOnDisk.Add(SlotName, InSaveData);
	return true;
}

void UMockSaveGameSerializer::AsyncSaveGameToSlot(USaveGame& SaveGameObject, const FSlotName& SlotName, const int32 UserIndex, FOnAsyncSaveCompleted Callback)
{
	const bool bSuccess = TrySaveGameToSlot(SaveGameObject, SlotName, UserIndex);;
	Callback.ExecuteIfBound(SlotName, UserIndex, bSuccess);
}

bool UMockSaveGameSerializer::TryLoadDataFromSlot(const FSlotName& SlotName, const int32 UserIndex, TArray<uint8>& OutSaveData)
{
	if (!PretendedSaveGamesOnDisk.Contains(SlotName))
		return false;

	OutSaveData = PretendedSaveGamesOnDisk[SlotName];
	return true;
}

void UMockSaveGameSerializer::AsyncLoadGameFromSlot(const FSlotName& SlotName, const int32 UserIndex, FOnAsyncLoadCompleted Callback)
{
	USaveGame* SaveGame = nullptr;
	TryLoadGameFromSlot(SlotName, UserIndex, OUT SaveGame);
	Callback.ExecuteIfBound(SlotName, UserIndex, SaveGame);
}

bool UMockSaveGameSerializer::TryDeleteGameInSlot(const FSlotName& SlotName, const int32 UserIndex)
{
	return (PretendedSaveGamesOnDisk.Remove(SlotName) > 0);
}
