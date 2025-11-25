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
#include "AutosaveCheckpoint.h"
#include "Components/ActorComponent.h"

#include "AutosaveCheckpointComponent.generated.h"

/**
 * Automatically spawns an AAutostartCheckpoint actor and attaches it to this component.
 * Checkpoints are never spatially loaded, but the owning actor could be. In this case, the checkpoint will not be attached
 * to this component, but instead be moved to an "AutosaveCheckpoints" folder in the scene outliner.
 * The linked checkpoint will have an auto-generated PlayerStartTag based on the name of this component's owning actor.
 * While most of this component's logic is editor-only, it also provides runtime utility to autosave the game at the linked checkpoint.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class WEEKENDSAVEGAME_API UAutosaveCheckpointComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAutosaveCheckpointComponent();

	/** Requests an autosave and saves the linked checkpoints PlayerStartTag into the ModularSaveGame, so the player can be restored here. */
	UFUNCTION(BlueprintCallable, Category = "Autosave Checkpoint")
	void RequestAutosaveAtCheckpoint();

	/** @returns the linked AutosaveCheckpoint actor. */
	UFUNCTION(BlueprintCallable, Category = "Autosave Checkpoint")
	AAutosaveCheckpoint* GetAutosaveCheckpoint() const;

	// - UActorComponent
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	// --

protected:
	/** Class of the checkpoint actor to automatically spawn and link to this component. */
	UPROPERTY(EditAnywhere, NoClear, Category = "Autosave Checkpoint", meta = (OnlyPlaceable))
	TSubclassOf<AAutosaveCheckpoint> CheckpointClass = AAutosaveCheckpoint::StaticClass();

	/** Soft link to our checkpoint actor. The checkpoint is never spatially loaded, but this actor could be. */
	UPROPERTY(EditInstanceOnly, Category = "Autosave Checkpoint")
	TSoftObjectPtr<AAutosaveCheckpoint> LinkedCheckpoint = nullptr;

	virtual void FindOrCreateCheckpoint();

#if WITH_EDITOR
	virtual void DespawnCheckpoint();

	UFUNCTION(CallInEditor, Category = "Autosave Checkpoint")
	virtual void RespawnCheckpoint();

	virtual void RenameCheckpoint();
	virtual void HandleEditorActorMoved(AActor* MovedActor);
	virtual void HandleLevelActorDeleted(AActor* DeletedActor);
	virtual void HandleActorLabelChanged(AActor* RenamedActor);
#endif
};
