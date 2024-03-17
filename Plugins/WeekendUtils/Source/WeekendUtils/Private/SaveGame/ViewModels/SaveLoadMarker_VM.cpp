///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "SaveGame/ViewModels/SaveLoadMarker_VM.h"

#include "SaveGame/SaveGameService.h"

USaveLoadMarker_VM::USaveLoadMarker_VM()
{
	ServiceDependencies.Add<USaveGameService>();
}

FTimespan USaveLoadMarker_VM::GetTimeSinceLastSave() const
{
	return (FDateTime::UtcNow() - UtcTimeOfLastSave);
}

bool USaveLoadMarker_VM::ShouldShowTimeSinceLastSave() const
{
	return SaveGameService.IsValid()
		&& SaveGameService->GetCurrentSaveGame().GetUtcTimeOfLastSave().IsSet();
}

void USaveLoadMarker_VM::BeginUsage()
{
	SaveGameService = UseGameServiceAsWeakPtr<USaveGameService>(this);
	SaveGameService->OnStatusChanged.AddUObject(this, &ThisClass::UpdateForStatus);
	UpdateForStatus(SaveGameService->GetCurrentStatus());
}

void USaveLoadMarker_VM::EndUsage()
{
	if (SaveGameService.IsValid())
	{
		SaveGameService->OnStatusChanged.RemoveAll(this);
		SaveGameService.Reset();
	}

	UE_MVVM_SET_PROPERTY_VALUE(bIsLoading, false);
	UE_MVVM_SET_PROPERTY_VALUE(bIsSaving, false);
	UE_MVVM_SET_PROPERTY_VALUE(bIsSavingOrLoading, true);
	UE_MVVM_SET_PROPERTY_VALUE(SuggestedWidgetVisibility, ESlateVisibility::Collapsed);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ShouldShowTimeSinceLastSave);
}

void USaveLoadMarker_VM::BeginDestroy()
{
	EndUsage();

	Super::BeginDestroy();
}

void USaveLoadMarker_VM::UpdateForStatus(USaveGameService::EStatus NewStatus)
{
	switch (NewStatus)
	{
		case USaveGameService::EStatus::Saving:
		{
			UE_MVVM_SET_PROPERTY_VALUE(bIsLoading, false);
			UE_MVVM_SET_PROPERTY_VALUE(bIsSaving, true);
			UE_MVVM_SET_PROPERTY_VALUE(bIsSavingOrLoading, true);
			UE_MVVM_SET_PROPERTY_VALUE(SuggestedWidgetVisibility, ESlateVisibility::SelfHitTestInvisible);
			return;
		}

		case USaveGameService::EStatus::Loading:
		{
			UE_MVVM_SET_PROPERTY_VALUE(bIsLoading, true);
			UE_MVVM_SET_PROPERTY_VALUE(bIsSaving, false);
			UE_MVVM_SET_PROPERTY_VALUE(bIsSavingOrLoading, true);
			UE_MVVM_SET_PROPERTY_VALUE(SuggestedWidgetVisibility, ESlateVisibility::SelfHitTestInvisible);
			return;
		}

		case USaveGameService::EStatus::Idle:
		{
			const FCurrentSaveGame& CurrentSaveGame = SaveGameService->GetCurrentSaveGame();
			UE_MVVM_SET_PROPERTY_VALUE(bIsLoading, false);
			UE_MVVM_SET_PROPERTY_VALUE(bIsSaving, false);
			UE_MVVM_SET_PROPERTY_VALUE(bIsSavingOrLoading, false);
			UE_MVVM_SET_PROPERTY_VALUE(UtcTimeOfLastSave, CurrentSaveGame.GetUtcTimeOfLastSave().Get(FDateTime::UtcNow()));
			UE_MVVM_SET_PROPERTY_VALUE(SuggestedWidgetVisibility, ESlateVisibility::Collapsed);
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetTimeSinceLastSave);
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ShouldShowTimeSinceLastSave);
			return;
		}

		default:
		case USaveGameService::EStatus::Uninitialized:
		{
			UE_MVVM_SET_PROPERTY_VALUE(bIsLoading, false);
			UE_MVVM_SET_PROPERTY_VALUE(bIsSaving, false);
			UE_MVVM_SET_PROPERTY_VALUE(bIsSavingOrLoading, true);
			UE_MVVM_SET_PROPERTY_VALUE(SuggestedWidgetVisibility, ESlateVisibility::Collapsed);
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ShouldShowTimeSinceLastSave);
		}
	}
}
