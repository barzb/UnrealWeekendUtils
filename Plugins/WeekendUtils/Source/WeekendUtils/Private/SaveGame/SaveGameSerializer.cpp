///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "SaveGame/SaveGameSerializer.h"

#include "PlatformFeatures.h"
#include "SaveGameSystem.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/GameplayStatics.h"

///////////////////////////////////////////////////////////////////////////////////////

FWeekendUtilsSaveGameProxyArchive::FWeekendUtilsSaveGameProxyArchive(FArchive& InInnerArchive, USaveGame& InSaveGame, bool bInLoadIfFindFails) :
	FObjectAndNameAsStringProxyArchive(InInnerArchive, bInLoadIfFindFails), SaveGame(InSaveGame)
{
	ArIsSaveGame = true;
}

FArchive& FWeekendUtilsSaveGameProxyArchive::operator<<(UObject*& Obj)
{
	if (IsLoading())
	{
		FString ObjectPath, ClassPath, IsSubObjectOfSaveGame;
		InnerArchive << ObjectPath;
		InnerArchive << ClassPath;
		InnerArchive << IsSubObjectOfSaveGame;

		if (IsSubObjectOfSaveGame != "1")
		{
			// Find/load objects from the asset registry (= asset pointers):
			Obj = FindObject<UObject>(nullptr, *ObjectPath, false);
			if (!Obj && bLoadIfFindFails)
			{
				Obj = LoadObject<UObject>(nullptr, *ObjectPath);
			}
		}
		else
		{
			// Reconstruct subobjects that were part of the save game hierarchy:
			const UClass* Class = UClass::TryFindTypeSlow<UClass>(ClassPath);
			if (!Class && bLoadIfFindFails)
			{
				Class = LoadObject<UClass>(nullptr, *ClassPath);
			}
			if (Class)
			{
				Obj = NewObject<UObject>(&SaveGame, Class);
				Obj->Serialize(InnerArchive);
			}
		}
	}
	else
	{
		FString ObjectPath(GetPathNameSafe(Obj));
		FString ClassPath (GetPathNameSafe(Obj ? Obj->GetClass() : nullptr));
		FString IsSubObjectOfSaveGame = Obj->IsInOuter(&SaveGame) ? "1" : "0";
		InnerArchive << ObjectPath;
		InnerArchive << ClassPath;
		InnerArchive << IsSubObjectOfSaveGame;
		if (Obj->IsInOuter(&SaveGame))
		{
			Obj->Serialize(InnerArchive);
		}
	}
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////

bool USaveGameSerializer::TrySerializeSaveGame(USaveGame& InSaveGameObject, TArray<uint8>& OutSaveData) const
{
	// (i) FWeekendUtilsSaveGameProxyArchive is not used here, because the base implementation of the USaveGameSerializer
	// will just forward all calls to the default UE UGameplayStatics implementation. But see UModularSaveGameSerializer.
	return UGameplayStatics::SaveGameToMemory(&InSaveGameObject, OUT OutSaveData);
}

bool USaveGameSerializer::TryDeserializeSaveGame(const TArray<uint8>& InSaveData, USaveGame*& OutSaveGameObject) const
{
	// (i) FWeekendUtilsSaveGameProxyArchive is not used here, because the base implementation of the USaveGameSerializer
	// will just forward all calls to the default UE UGameplayStatics implementation. But see UModularSaveGameSerializer.
	OutSaveGameObject = UGameplayStatics::LoadGameFromMemory(InSaveData);
	return (OutSaveGameObject != nullptr);
}

bool USaveGameSerializer::DoesSaveGameExist(const FSlotName& SlotName, const int32 UserIndex) const
{
	return UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex);
}

bool USaveGameSerializer::TrySaveDataToSlot(const TArray<uint8>& InSaveData, const FSlotName& SlotName, const int32 UserIndex)
{
	return UGameplayStatics::SaveDataToSlot(InSaveData, SlotName, UserIndex);
}

bool USaveGameSerializer::TrySaveGameToSlot(USaveGame& SaveGameObject, const FSlotName& SlotName, const int32 UserIndex)
{
	if (TArray<uint8> ObjectBytes; TrySerializeSaveGame(SaveGameObject, ObjectBytes))
	{
		return TrySaveDataToSlot(ObjectBytes, SlotName, UserIndex);
	}
	return false;
}

void USaveGameSerializer::AsyncSaveGameToSlot(USaveGame& SaveGameObject, const FSlotName& SlotName, const int32 UserIndex, FOnAsyncSaveCompleted Callback)
{
	// (i) Mostly copied from UGameplayStatics::AsyncSaveGameToSlot,
	// but using this serializers internal methods.

	ISaveGameSystem* SaveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
	TSharedRef<TArray<uint8>> ObjectBytes(new TArray<uint8>());

	if (SaveSystem && (SlotName.Len() > 0) && 
		TrySerializeSaveGame(SaveGameObject, OUT *ObjectBytes) && (ObjectBytes->Num() > 0) )
	{
		const FPlatformUserId PlatformUserId = FPlatformMisc::GetPlatformUserForUserIndex(UserIndex);
		SaveSystem->SaveGameAsync(false, *SlotName, PlatformUserId, ObjectBytes, 
			[Callback, UserIndex](const FString& ResultSlotName, FPlatformUserId, bool bSuccess)
			{
				check(IsInGameThread());
				Callback.ExecuteIfBound(ResultSlotName, UserIndex, bSuccess);
			}
		);
	}
	else
	{
		Callback.ExecuteIfBound(SlotName, UserIndex, false);
	}
}

bool USaveGameSerializer::TryLoadDataFromSlot(const FSlotName& SlotName, const int32 UserIndex, TArray<uint8>& OutSaveData)
{
	return UGameplayStatics::LoadDataFromSlot(OUT OutSaveData, SlotName, UserIndex);
}

bool USaveGameSerializer::TryLoadGameFromSlot(const FSlotName& SlotName, const int32 UserIndex, USaveGame*& OutSaveGameObject)
{
	if (TArray<uint8> ObjectBytes; TryLoadDataFromSlot(SlotName, UserIndex, OUT ObjectBytes))
	{
		return TryDeserializeSaveGame(ObjectBytes, OUT OutSaveGameObject);
	}
	return false;
}

void USaveGameSerializer::AsyncLoadGameFromSlot(const FSlotName& SlotName, const int32 UserIndex, FOnAsyncLoadCompleted Callback)
{
	// (i) Mostly copied from UGameplayStatics::AsyncLoadGameFromSlot,
	// but using this serializers internal methods.

	ISaveGameSystem* SaveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
	if (SaveSystem && (SlotName.Len() > 0))
	{
		const FPlatformUserId PlatformUserId = FPlatformMisc::GetPlatformUserForUserIndex(UserIndex);
		SaveSystem->LoadGameAsync(false, *SlotName, PlatformUserId,
			[this, Callback, UserIndex](const FString& ResultSlotName, FPlatformUserId, bool bSuccess, const TArray<uint8>& Data)
			{
				check(IsInGameThread());

				USaveGame* LoadedGame = nullptr;
				if (bSuccess)
				{
					TryDeserializeSaveGame(Data, OUT LoadedGame);
				}

				Callback.ExecuteIfBound(ResultSlotName, UserIndex, LoadedGame);
			}
		);
	}
	else
	{
		Callback.ExecuteIfBound(SlotName, UserIndex, nullptr);
	}
}

bool USaveGameSerializer::TryDeleteGameInSlot(const FSlotName& SlotName, const int32 UserIndex)
{
	return UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);
}
