///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "SaveGame/Modules/LevelObjectRestorer.h"

#include "Engine/World.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

namespace
{
	void CheckLevelObject(const UObject& Object)
	{
		checkf(IsValid(&Object),
			TEXT("USaveGameModule_LevelObjects: Invalid object: %s"), *GetNameSafe(&Object));
		checkf(Object.GetWorld() && Object.GetWorld()->IsGameWorld(),
			TEXT("USaveGameModule_LevelObjects: Non-level-objects are not supported: %s"), *GetNameSafe(&Object));
		checkf(!Object.HasAnyFlags(RF_ArchetypeObject | RF_ClassDefaultObject),
			TEXT("USaveGameModule_LevelObjects: CDO objects are not supported: %s"), *GetNameSafe(&Object));
		checkf(!Object.HasAnyFlags(RF_Standalone | RF_Transient),
			TEXT("USaveGameModule_LevelObjects: Persistent objects are not supported: %s"), *GetNameSafe(&Object));
	}
}

void ULevelObjectRestorer::RegisterLevelObject(UObject& Object, TOptional<FString> CustomUniqueObjectId, bool bImmediatelyRestoreIfPossible)
{
	CheckLevelObject(Object);
	const TWeakObjectPtr<> ObjectPtr = MakeWeakObjectPtr(&Object);
	ensureMsgf(!SimpleRegisteredObjects.Contains(ObjectPtr), TEXT("%s is already registered"), *Object.GetName());
	SimpleRegisteredObjects.Add(ObjectPtr);

	const FString ObjectId = CustomUniqueObjectId.Get(Object.GetPathName());
	UniqueIdsOfRegisteredObjects.Add(ObjectPtr, ObjectId);

	if (bImmediatelyRestoreIfPossible && ObjectStates.Contains(ObjectId))
	{
		RestoreObjectFromState(ObjectStates[ObjectId], false, IN OUT Object);
	}
	else
	{
		FLevelObjectSaveGameState& State = ObjectStates.FindOrAdd(ObjectId);
		SaveObjectToState(Object, false, IN OUT State);
	}
}

void ULevelObjectRestorer::RegisterLevelObjectWithTransform(AActor& Actor, TOptional<FString> CustomUniqueObjectId, bool bImmediatelyRestoreIfPossible)
{
	CheckLevelObject(Actor);
	const TWeakObjectPtr<> ObjectPtr = MakeWeakObjectPtr(&Actor);
	ensureMsgf(!RegisteredObjectsWithTransform.Contains(ObjectPtr), TEXT("%s is already registered"), *Actor.GetName());
	RegisteredObjectsWithTransform.Add(ObjectPtr);

	const FString ObjectId = CustomUniqueObjectId.Get(Actor.GetPathName());
	UniqueIdsOfRegisteredObjects.Add(ObjectPtr, ObjectId);

	if (bImmediatelyRestoreIfPossible && ObjectStates.Contains(ObjectId))
	{
		RestoreObjectFromState(ObjectStates[ObjectId], true, IN OUT Actor);
	}
	else
	{
		FLevelObjectSaveGameState& State = ObjectStates.FindOrAdd(ObjectId);
		SaveObjectToState(Actor, true, IN OUT State);
	}
}

void ULevelObjectRestorer::RegisterLevelObjectWithTransform(USceneComponent& SceneComponent, TOptional<FString> CustomUniqueObjectId, bool bImmediatelyRestoreIfPossible)
{
	CheckLevelObject(SceneComponent);
	const TWeakObjectPtr<> ObjectPtr = MakeWeakObjectPtr(&SceneComponent);
	ensureMsgf(!RegisteredObjectsWithTransform.Contains(ObjectPtr), TEXT("%s is already registered"), *SceneComponent.GetName());
	RegisteredObjectsWithTransform.Add(ObjectPtr);

	const FString ObjectId = CustomUniqueObjectId.Get(SceneComponent.GetPathName());
	UniqueIdsOfRegisteredObjects.Add(ObjectPtr, ObjectId);

	if (bImmediatelyRestoreIfPossible && ObjectStates.Contains(ObjectId))
	{
		RestoreObjectFromState(ObjectStates[ObjectId], true, IN OUT SceneComponent);
	}
	else
	{
		FLevelObjectSaveGameState& State = ObjectStates.FindOrAdd(ObjectId);
		SaveObjectToState(SceneComponent, true, IN OUT State);
	}
}

void ULevelObjectRestorer::UnregisterLevelObject(UObject& Object, TOptional<FString> CustomUniqueObjectId, bool bKeepObjectState)
{
	CheckLevelObject(Object);
	const TWeakObjectPtr<> ObjectPtr = MakeWeakObjectPtr(&Object);
	ensureMsgf(SimpleRegisteredObjects.Contains(ObjectPtr), TEXT("%s is not registered"), *Object.GetName());
	SimpleRegisteredObjects.Remove(ObjectPtr);
	UniqueIdsOfRegisteredObjects.Remove(ObjectPtr);

	const FString ObjectId = CustomUniqueObjectId.Get(Object.GetPathName());
	if (bKeepObjectState)
	{
		FLevelObjectSaveGameState& State = ObjectStates.FindOrAdd(ObjectId);
		SaveObjectToState(Object, false, IN OUT State);
	}
	else
	{
		ObjectStates.Remove(ObjectId);
	}
}

void ULevelObjectRestorer::UnregisterLevelObjectWithTransform(AActor& Actor, TOptional<FString> CustomUniqueObjectId, bool bKeepObjectState)
{
	CheckLevelObject(Actor);
	const TWeakObjectPtr<> ObjectPtr = MakeWeakObjectPtr(&Actor);
	ensureMsgf(RegisteredObjectsWithTransform.Contains(ObjectPtr), TEXT("%s is not registered"), *Actor.GetName());
	RegisteredObjectsWithTransform.Remove(ObjectPtr);
	UniqueIdsOfRegisteredObjects.Remove(ObjectPtr);

	const FString ObjectId = CustomUniqueObjectId.Get(Actor.GetPathName());
	if (bKeepObjectState)
	{
		FLevelObjectSaveGameState& State = ObjectStates.FindOrAdd(ObjectId);
		SaveObjectToState(Actor, true, IN OUT State);
	}
	else
	{
		ObjectStates.Remove(ObjectId);
	}
}

void ULevelObjectRestorer::UnregisterLevelObjectWithTransform(USceneComponent& SceneComponent, TOptional<FString> CustomUniqueObjectId, bool bKeepObjectState)
{
	CheckLevelObject(SceneComponent);
	const TWeakObjectPtr<> ObjectPtr = MakeWeakObjectPtr(&SceneComponent);
	ensureMsgf(RegisteredObjectsWithTransform.Contains(ObjectPtr), TEXT("%s is not registered"), *SceneComponent.GetName());
	RegisteredObjectsWithTransform.Remove(ObjectPtr);
	UniqueIdsOfRegisteredObjects.Remove(ObjectPtr);

	const FString ObjectId = CustomUniqueObjectId.Get(SceneComponent.GetPathName());
	if (bKeepObjectState)
	{
		FLevelObjectSaveGameState& State = ObjectStates.FindOrAdd(ObjectId);
		SaveObjectToState(SceneComponent, true, IN OUT State);
	}
	else
	{
		ObjectStates.Remove(ObjectId);
	}
}

void ULevelObjectRestorer::Serialize(FArchive& Ar)
{
	for (TWeakObjectPtr<> RegisteredObject : SimpleRegisteredObjects.Union(RegisteredObjectsWithTransform))
	{
		if (!RegisteredObject.IsValid())
			continue;

		UObject* Object = RegisteredObject.Get();
		const FString& ObjectId = UniqueIdsOfRegisteredObjects[RegisteredObject];
		const bool bHasTransform = RegisteredObjectsWithTransform.Contains(RegisteredObject);
		FLevelObjectSaveGameState& State = ObjectStates.FindOrAdd(ObjectId);
		if (Ar.IsSaving())
		{
			SaveObjectToState(*Object, bHasTransform, IN OUT State);
		}
		else
		{
			RestoreObjectFromState(State, bHasTransform, IN OUT *Object);
		}
	}

	Super::Serialize(Ar);
}

void ULevelObjectRestorer::SaveObjectToState(UObject& Object, bool bSaveTransform, FLevelObjectSaveGameState& InOutState) const
{
	FMemoryWriter MemWriter(InOutState.ByteData);
	if (bSaveTransform)
	{
		FTransform Transform = GetObjectTransform(Object);
		MemWriter << Transform;
	}

	FObjectAndNameAsStringProxyArchive Archive(MemWriter, true);
	Archive.ArIsSaveGame = true;
	Object.Serialize(Archive);
	InOutState.ByteDataSize = FMath::Min(InOutState.ByteData.Num(), INT32_MAX);
}

void ULevelObjectRestorer::RestoreObjectFromState(const FLevelObjectSaveGameState& State, bool bRestoreTransform, UObject& InOutObject) const
{
	FMemoryReader MemReader(State.ByteData);
	if (bRestoreTransform)
	{
		FTransform Transform = FTransform::Identity;
		MemReader << Transform;
		SetObjectTransform(InOutObject, Transform);
	}

	FObjectAndNameAsStringProxyArchive Archive(MemReader, true);
	Archive.ArIsSaveGame = true;
	InOutObject.Serialize(Archive);
}

FTransform ULevelObjectRestorer::GetObjectTransform(UObject& Object) const
{
	if (const AActor* Actor = Cast<AActor>(&Object))
		return Actor->GetTransform();

	if (const USceneComponent* SceneComponent = Cast<USceneComponent>(&Object))
		return SceneComponent->GetComponentTransform();

	return FTransform::Identity;
}

void ULevelObjectRestorer::SetObjectTransform(UObject& Object, const FTransform& Transform) const
{
	if (AActor* Actor = Cast<AActor>(&Object))
	{
		Actor->SetActorTransform(Transform);
	}
	else if (USceneComponent* SceneComponent = Cast<USceneComponent>(&Object))
	{
		SceneComponent->SetWorldTransform(Transform);
	}
}
