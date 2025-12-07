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
#include "UObject/Object.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#include "SaveGameSerializer.generated.h"

class USaveGame;

///////////////////////////////////////////////////////////////////////////////////////

/**
 * Extends a proxy archive that serializes UObjects and FNames as string data.
 * Recursively serializes sub-objects nested inside serialized objects and restores
 * them by allocating them via NewObject<T>().
 */
struct WEEKENDSAVEGAME_API FWeekendUtilsSubobjectProxyArchive : FObjectAndNameAsStringProxyArchive
{
	FWeekendUtilsSubobjectProxyArchive(FArchive& InInnerArchive, UObject& InSubobjectOwner, bool bInLoadIfFindFails = true);
	virtual FArchive& operator<<(UObject*& Obj) override;
	UObject& SubobjectOwner;
};

///////////////////////////////////////////////////////////////////////////////////////

/**
 * Polymorphic sub-object of @USaveGameService that extracts implementation details of
 * SaveGame serialization, deserialization, and save file management.
 * The base implementation forwards (or copies) calls of @UGameplayStatics.
 */
UCLASS()
class WEEKENDSAVEGAME_API USaveGameSerializer : public UObject
{
	GENERATED_BODY()

public:
	using FSlotName = FString;

	DECLARE_DELEGATE_ThreeParams(FOnAsyncSaveCompleted, const FSlotName&, const int32, bool);
	DECLARE_DELEGATE_ThreeParams(FOnAsyncLoadCompleted, const FSlotName&, const int32, USaveGame*);

	virtual bool TrySerializeSaveGame(USaveGame& InSaveGameObject, TArray<uint8>& OutSaveData) const;
	virtual bool TryDeserializeSaveGame(const TArray<uint8>& InSaveData, USaveGame*& OutSaveGameObject) const;

	virtual bool DoesSaveGameExist(const FSlotName& SlotName, const int32 UserIndex) const;

	virtual bool TrySaveDataToSlot(const TArray<uint8>& InSaveData, const FSlotName& SlotName, const int32 UserIndex);
	virtual bool TrySaveGameToSlot(USaveGame& SaveGameObject, const FSlotName& SlotName, const int32 UserIndex);
	virtual void AsyncSaveGameToSlot(USaveGame& SaveGameObject, const FSlotName& SlotName, const int32 UserIndex, FOnAsyncSaveCompleted Callback);

	virtual bool TryLoadDataFromSlot(const FSlotName& SlotName, const int32 UserIndex, TArray<uint8>& OutSaveData);
	virtual bool TryLoadGameFromSlot(const FSlotName& SlotName, const int32 UserIndex, USaveGame*& OutSaveGameObject);
	virtual void AsyncLoadGameFromSlot(const FSlotName& SlotName, const int32 UserIndex, FOnAsyncLoadCompleted Callback);

	virtual bool TryDeleteGameInSlot(const FSlotName& SlotName, const int32 UserIndex, TOptional<FString> OptionalBackupFolder = {});
};
