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
#include "Components/SlateWrapperTypes.h"
#include "GameService/GameServiceUser.h"
#include "SaveGame/SaveGameService.h"

#include "SaveLoadMarker_VM.generated.h"

/**
 * View Model for a marker widget showing when the game is being saved or loaded.
 * @BeginUsage and @EndUsage must be called manually by the owning view.
 */
UCLASS()
class WEEKENDUTILS_API USaveLoadMarker_VM : public UMVVMViewModelBase,
											public FGameServiceUser
{
	GENERATED_BODY()

public:
	USaveLoadMarker_VM();

	UPROPERTY(FieldNotify, BlueprintReadOnly)
	ESlateVisibility SuggestedWidgetVisibility = ESlateVisibility::Collapsed;

	UPROPERTY(FieldNotify, BlueprintReadOnly)
	bool bIsSavingOrLoading = false;

	UPROPERTY(FieldNotify, BlueprintReadOnly)
	bool bIsLoading = false;

	UPROPERTY(FieldNotify, BlueprintReadOnly)
	bool bIsSaving = false;

	/** Timestamp (UTC) of the last time the game was saved. */
	UPROPERTY(FieldNotify, BlueprintReadOnly)
	FDateTime UtcTimeOfLastSave = FDateTime();

	/** @returns how long ago the current savegame was saved. */
	UFUNCTION(FieldNotify, BlueprintPure)
	FTimespan GetTimeSinceLastSave() const;

	UFUNCTION(FieldNotify, BlueprintPure)
	bool ShouldShowTimeSinceLastSave() const;

	UFUNCTION(BlueprintCallable)
	virtual void BeginUsage();

	UFUNCTION(BlueprintCallable)
	virtual void EndUsage();

protected:
	TWeakObjectPtr<USaveGameService> SaveGameService = nullptr;

	// - UObject
	virtual void BeginDestroy() override;
	// --

	virtual void UpdateForStatus(USaveGameService::EStatus NewStatus);
};
