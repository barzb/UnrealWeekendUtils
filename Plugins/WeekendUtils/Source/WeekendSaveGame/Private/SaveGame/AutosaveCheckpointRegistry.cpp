///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "SaveGame/AutosaveCheckpointRegistry.h"

#include "EngineUtils.h"
#include "Engine/World.h"
#include "SaveGame/AutosaveCheckpoint.h"

UAutosaveCheckpointRegistry* UAutosaveCheckpointRegistry::Get(const UObject* ContextObject)
{
	return UWorld::GetSubsystem<UAutosaveCheckpointRegistry>(ContextObject ? ContextObject->GetWorld() : nullptr);
}

void UAutosaveCheckpointRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	GatherCheckpointsInWorld();
}

AAutosaveCheckpoint* UAutosaveCheckpointRegistry::FindCheckpoint(FName PlayerStartTag)
{
	if (AAutosaveCheckpoint* Checkpoint = GetRegisteredCheckpoint(PlayerStartTag))
		return Checkpoint;

	GatherCheckpointsInWorld();
	return GetRegisteredCheckpoint(PlayerStartTag);
}

AAutosaveCheckpoint* UAutosaveCheckpointRegistry::GetRegisteredCheckpoint(FName PlayerStartTag) const
{
	auto* FoundCheckpoint = RegisteredCheckpoints.FindByPredicate([PlayerStartTag](AAutosaveCheckpoint* Checkpoint)
	{
		return (IsValid(Checkpoint) && Checkpoint->PlayerStartTag == PlayerStartTag);
	});
	return FoundCheckpoint ? *FoundCheckpoint : nullptr;
}

void UAutosaveCheckpointRegistry::RegisterCheckpoint(AAutosaveCheckpoint& Checkpoint)
{
	ensureAlwaysMsgf(!RegisteredCheckpoints.Contains(&Checkpoint),
			TEXT("Checkpoint %s is already registered"), *Checkpoint.GetHumanReadableName());

	RegisteredCheckpoints.AddUnique(&Checkpoint);
}

void UAutosaveCheckpointRegistry::UnregisterCheckpoint(AAutosaveCheckpoint& Checkpoint)
{
	if (const int32 Index = RegisteredCheckpoints.IndexOfByKey(&Checkpoint); Index != INDEX_NONE)
	{
		RegisteredCheckpoints.RemoveAt(Index);
	}
}

void UAutosaveCheckpointRegistry::GatherCheckpointsInWorld()
{
	RegisteredCheckpoints.Empty();
	for (AAutosaveCheckpoint* Checkpoint : TActorRange<AAutosaveCheckpoint>(GetWorld()))
	{
		ensureAlwaysMsgf(!RegisteredCheckpoints.Contains(Checkpoint),
			TEXT("Checkpoint \"%s\" is already registered"), *Checkpoint->GetHumanReadableName());

		RegisteredCheckpoints.AddUnique(Checkpoint);
	}
}
