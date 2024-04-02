///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "SaveGame/ModularSaveGame.h"

#include "GameService/GameServiceLocator.h"
#include "SaveGame/SaveGameHeader.h"
#include "SaveGame/SaveGameService.h"

///////////////////////////////////////////////////////////////////////////////////////
/// @UModularSaveGame

const UModularSaveGame* UModularSaveGame::GetCurrent()
{
	const USaveGameService* SaveGameService = UGameServiceLocator::FindService<USaveGameService>();
	if (!SaveGameService)
		return nullptr;

	return SaveGameService->GetCurrentSaveGame().GetPtr<UModularSaveGame>();
}

UModularSaveGame* UModularSaveGame::GetMutableCurrent()
{
	const USaveGameService* SaveGameService = UGameServiceLocator::FindService<USaveGameService>();
	if (!SaveGameService)
		return nullptr;

	return SaveGameService->GetCurrentSaveGame().GetMutablePtr<UModularSaveGame>();
}

///////////////////////////////////////////////////////////////////////////////////////
/// SAVE GAME HEADER - Mostly copied from UE/GameplayStatic.cpp

FModularSaveGameHeader::FModularSaveGameHeader() :
	FileTypeTag(0),
	SaveGameFileVersion(0),
	CustomVersionFormat(static_cast<int32>(ECustomVersionSerializationFormat::Unknown))
{
}

FModularSaveGameHeader::FModularSaveGameHeader(TSubclassOf<UModularSaveGame> ObjectType, const FInstancedStruct& HeaderData) :
	FileTypeTag(MODULAR_SAVEGAME_FILE_TYPE_TAG),
	SaveGameFileVersion(MODULAR_SAVEGAME_FILE_VERSION),
	PackageFileUEVersion(GPackageFileUEVersion),
	SavedEngineVersion(FEngineVersion::Current()),
	CustomVersionFormat(static_cast<int32>(ECustomVersionSerializationFormat::Latest)),
	CustomVersions(FCurrentCustomVersions::GetAll()),
	SaveGameClassName(ObjectType->GetPathName()),
	CustomHeaderData(HeaderData)
{
}

void FModularSaveGameHeader::Clear()
{
	FileTypeTag = 0;
	SaveGameFileVersion = 0;
	PackageFileUEVersion.Reset();
	SavedEngineVersion.Empty();
	CustomVersionFormat = static_cast<int32>(ECustomVersionSerializationFormat::Unknown);
	CustomVersions.Empty();
	SaveGameClassName.Empty();
	CustomHeaderData.Reset();
}

bool FModularSaveGameHeader::TryRead(FMemoryReader& MemoryReader)
{
	Clear();

	// Check incompatible save file type:
	MemoryReader << FileTypeTag;
	if (FileTypeTag != MODULAR_SAVEGAME_FILE_TYPE_TAG)
	{
		MemoryReader.Seek(0);
		return false;
	}

	// Check incompatible save file version:
	MemoryReader << SaveGameFileVersion;
	if (SaveGameFileVersion < MODULAR_SAVEGAME_FILE_VERSION)
	{
		MemoryReader.Seek(0);
		return false;
	}

	// Read engine and UE version information:
	MemoryReader << PackageFileUEVersion;
	MemoryReader << SavedEngineVersion;
	MemoryReader.SetUEVer(PackageFileUEVersion);
	MemoryReader.SetEngineVer(SavedEngineVersion);

	// Read custom version data:
	MemoryReader << CustomVersionFormat;
	CustomVersions.Serialize(MemoryReader, static_cast<ECustomVersionSerializationFormat::Type>(CustomVersionFormat));
	MemoryReader.SetCustomVersions(CustomVersions);

	// Read out custom header data:
	MemoryReader << SaveGameClassName;
	FObjectAndNameAsStringProxyArchive ProxyArchive(MemoryReader, true);
	CustomHeaderData.Serialize(ProxyArchive);

	return true;
}

bool FModularSaveGameHeader::TryWrite(FMemoryWriter& MemoryWriter)
{
	// Write file type tag that identifies this file type:
	MemoryWriter << FileTypeTag;

	// Write version for this file format, for compatibility checks:
	MemoryWriter << SaveGameFileVersion;

	// Write out engine and UE version information:
	MemoryWriter << PackageFileUEVersion;
	MemoryWriter << SavedEngineVersion;

	// Write out custom version data:
	MemoryWriter << CustomVersionFormat;
	CustomVersions.Serialize(MemoryWriter, static_cast<ECustomVersionSerializationFormat::Type>(CustomVersionFormat));

	// Write custom header data:
	MemoryWriter << SaveGameClassName;
	FObjectAndNameAsStringProxyArchive ProxyArchive(MemoryWriter, true);
	CustomHeaderData.Serialize(ProxyArchive);

	return true;
}

#if WITH_EDITOR
void UModularSaveGame::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, Modules))
	{
		for (auto Itr = Modules.CreateIterator(); Itr; ++Itr)
		{
			if (!Itr.Key().IsNone() || !Itr.Value())
				continue;

			Itr.Key() = Itr.Value()->DefaultModuleName;
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////
/// @UModularSaveGameSerializer

bool UModularSaveGameSerializer::TrySerializeSaveGame(USaveGame& InSaveGameObject, TArray<uint8>& OutSaveData) const
{
	const UModularSaveGame* ModularSaveGame = Cast<UModularSaveGame>(&InSaveGameObject);
	FMemoryWriter MemoryWriter(OutSaveData, true);
	MemoryWriter.ArIsSaveGame = true;

	// Serialize header data:
	const FInstancedStruct CustomHeaderData = (ModularSaveGame && ModularSaveGame->GetInstancedHeaderData().IsValid())
		? *ModularSaveGame->GetInstancedHeaderData()
		: FInstancedStruct::Make<FSimpleSaveGameHeaderData>();
	FModularSaveGameHeader SaveHeader(InSaveGameObject.GetClass(), CustomHeaderData);
	if (!SaveHeader.TryWrite(MemoryWriter))
		return false;

	// Serialize the save game object and all supported properties:
	FWeekendUtilsSaveGameProxyArchive Archive(MemoryWriter, InSaveGameObject);
	InSaveGameObject.Serialize(Archive);

	return true;
}

bool UModularSaveGameSerializer::TryDeserializeSaveGame(const TArray<uint8>& InSaveData, USaveGame*& OutSaveGameObject) const
{
	OutSaveGameObject = nullptr;
	if (InSaveData.IsEmpty())
		return false;

	FMemoryReader MemoryReader(InSaveData, true);
	MemoryReader.ArIsSaveGame = true;

	// Restore header data:
	FModularSaveGameHeader SaveHeader;
	if (!SaveHeader.TryRead(MemoryReader))
		return false;

	// Restore the save game class info:
	const UClass* SaveGameClass = UClass::TryFindTypeSlow<UClass>(SaveHeader.SaveGameClassName);
	if (!SaveGameClass)
	{
		SaveGameClass = LoadObject<UClass>(nullptr, *SaveHeader.SaveGameClassName);
	}

	// Create (empty) save game object and then restore all of its saved properties:
	OutSaveGameObject = NewObject<USaveGame>(GetOuter(), SaveGameClass);
	FWeekendUtilsSaveGameProxyArchive Archive(MemoryReader, *OutSaveGameObject);
	OutSaveGameObject->Serialize(Archive);

	if (UModularSaveGame* ModularSaveGame = Cast<UModularSaveGame>(OutSaveGameObject))
	{
		ModularSaveGame->SetInstancedHeaderData(SaveHeader.CustomHeaderData);
	}

	return true;
}
