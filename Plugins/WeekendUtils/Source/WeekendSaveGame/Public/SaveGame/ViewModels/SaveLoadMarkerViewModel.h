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

#include "SaveLoadMarkerViewModel.generated.h"

/**
 * View Model for a marker widget showing when the game is being saved or loaded.
 * @BeginUsage and @EndUsage must be called manually by the owning view.
 */
UCLASS()
class WEEKENDSAVEGAME_API USaveLoadMarkerViewModel : public UMVVMViewModelBase,
													 public FGameServiceUser
{
	GENERATED_BODY()

public:
	USaveLoadMarkerViewModel();

	UPROPERTY(FieldNotify, BlueprintReadOnly, Category = "Weekend Utils|Save Game")
	ESlateVisibility SuggestedWidgetVisibility = ESlateVisibility::Collapsed;

	UPROPERTY(FieldNotify, BlueprintReadOnly, Category = "Weekend Utils|Save Game")
	bool bIsSavingOrLoading = false;

	UPROPERTY(FieldNotify, BlueprintReadOnly, Category = "Weekend Utils|Save Game")
	bool bIsLoading = false;

	UPROPERTY(FieldNotify, BlueprintReadOnly, Category = "Weekend Utils|Save Game")
	bool bIsSaving = false;

	/** Timestamp (UTC) of the last time the game was saved. */
	UPROPERTY(FieldNotify, BlueprintReadOnly, Category = "Weekend Utils|Save Game")
	FDateTime UtcTimeOfLastSave = FDateTime();

	/** @returns how long ago the current savegame was saved. */
	UFUNCTION(FieldNotify, BlueprintPure, Category = "Weekend Utils|Save Game")
	FTimespan GetTimeSinceLastSave() const;

	UFUNCTION(FieldNotify, BlueprintPure, Category = "Weekend Utils|Save Game")
	bool ShouldShowTimeSinceLastSave() const;

	UFUNCTION(BlueprintCallable, Category = "Weekend Utils|Save Game")
	virtual void BeginUsage();

	UFUNCTION(BlueprintCallable, Category = "Weekend Utils|Save Game")
	virtual void EndUsage();

protected:
	TWeakObjectPtr<USaveGameService> SaveGameService = nullptr;

	// - UObject
	virtual void BeginDestroy() override;
	// --

	virtual void UpdateForStatus(USaveGameService::EStatus NewStatus);
};