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
#include "Subsystems/WorldSubsystem.h"

#include "AutosaveCheckpointRegistry.generated.h"

class AAutosaveCheckpoint;

/**
 * Singleton registry for registering AutosaveCheckpoint actors at runtime, on-demand.
 * Offers API to find checkpoints by PlayerStartTag.
 */
UCLASS()
class WEEKENDSAVEGAME_API UAutosaveCheckpointRegistry : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UAutosaveCheckpointRegistry* Get(const UObject* ContextObject);

	// - UWorldSubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// --

	/** @returns - or attempts to locate - the checkpoint with given PlayerStartTag. */
	AAutosaveCheckpoint* FindCheckpoint(FName PlayerStartTag);

	/** @returns the registered checkpoint with given PlayerStartTag. */
	AAutosaveCheckpoint* GetRegisteredCheckpoint(FName PlayerStartTag) const;

	void RegisterCheckpoint(AAutosaveCheckpoint& Checkpoint);
	void UnregisterCheckpoint(AAutosaveCheckpoint& Checkpoint);

protected:
	void GatherCheckpointsInWorld();

	UPROPERTY()
	TArray<TObjectPtr<AAutosaveCheckpoint>> RegisteredCheckpoints = {};
};
