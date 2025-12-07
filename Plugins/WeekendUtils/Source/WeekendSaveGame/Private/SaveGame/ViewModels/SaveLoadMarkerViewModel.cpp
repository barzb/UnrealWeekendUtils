///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "SaveGame/ViewModels/SaveLoadMarkerViewModel.h"

#include "Containers/Ticker.h"
#include "SaveGame/SaveGameService.h"

FGameServiceUserConfig USaveLoadMarkerViewModel::ConfigureGameServiceUser() const
{
	return FGameServiceUserConfig(this)
		.AddServiceDependency<USaveGameService>();
}

void USaveLoadMarkerViewModel::BeginUsage()
{
	SaveGameService = UseGameServiceAsWeakPtr<USaveGameService>();
	SaveGameService->OnStatusChanged.AddUObject(this, &ThisClass::UpdateForStatus);
	UpdateForStatus(SaveGameService->GetCurrentStatus());
}

void USaveLoadMarkerViewModel::EndUsage()
{
	if (SaveGameService.IsValid())
	{
		SaveGameService->OnStatusChanged.RemoveAll(this);
		SaveGameService.Reset();
	}

	bIsShowingForMinShowtime = false;
	if (MinShowtimeHandle.IsSet())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(*MinShowtimeHandle);
		MinShowtimeHandle.Reset();
	}

	UE_MVVM_SET_PROPERTY_VALUE(bIsLoading, false);
	UE_MVVM_SET_PROPERTY_VALUE(bIsSaving, false);
	UE_MVVM_SET_PROPERTY_VALUE(bIsSavingOrLoading, true);
	UE_MVVM_SET_PROPERTY_VALUE(SuggestedWidgetVisibility, ESlateVisibility::Collapsed);
}

void USaveLoadMarkerViewModel::ConfigureMinimumShowtime(float MinShowtimeSeconds)
{
	ConfiguredMinShowtimeSeconds = MinShowtimeSeconds;
}

void USaveLoadMarkerViewModel::BeginDestroy()
{
	EndUsage();

	Super::BeginDestroy();
}

void USaveLoadMarkerViewModel::UpdateForStatus(USaveGameService::EStatus NewStatus)
{
	switch (NewStatus)
	{
		case USaveGameService::EStatus::Saving:
		{
			UE_MVVM_SET_PROPERTY_VALUE(bIsLoading, false);
			UE_MVVM_SET_PROPERTY_VALUE(bIsSaving, true);
			UE_MVVM_SET_PROPERTY_VALUE(bIsSavingOrLoading, true);
			UE_MVVM_SET_PROPERTY_VALUE(SuggestedWidgetVisibility, ESlateVisibility::SelfHitTestInvisible);

			StartMinShowTimer();
			break;
		}

		case USaveGameService::EStatus::Loading:
		{
			UE_MVVM_SET_PROPERTY_VALUE(bIsLoading, true);
			UE_MVVM_SET_PROPERTY_VALUE(bIsSaving, false);
			UE_MVVM_SET_PROPERTY_VALUE(bIsSavingOrLoading, true);
			UE_MVVM_SET_PROPERTY_VALUE(SuggestedWidgetVisibility, ESlateVisibility::SelfHitTestInvisible);

			StartMinShowTimer();
			break;
		}

		case USaveGameService::EStatus::Idle:
		{
			if (bIsShowingForMinShowtime)
				return;

			UE_MVVM_SET_PROPERTY_VALUE(bIsLoading, false);
			UE_MVVM_SET_PROPERTY_VALUE(bIsSaving, false);
			UE_MVVM_SET_PROPERTY_VALUE(bIsSavingOrLoading, false);
			UE_MVVM_SET_PROPERTY_VALUE(SuggestedWidgetVisibility, ESlateVisibility::Collapsed);
			break;
		}

		default:
		case USaveGameService::EStatus::Uninitialized:
		{
			UE_MVVM_SET_PROPERTY_VALUE(bIsLoading, false);
			UE_MVVM_SET_PROPERTY_VALUE(bIsSaving, false);
			UE_MVVM_SET_PROPERTY_VALUE(bIsSavingOrLoading, true);
			UE_MVVM_SET_PROPERTY_VALUE(SuggestedWidgetVisibility, ESlateVisibility::Collapsed);
		}
	}
}

void USaveLoadMarkerViewModel::StartMinShowTimer()
{
	if (!ConfiguredMinShowtimeSeconds.IsSet())
		return;

	bIsShowingForMinShowtime = true;
	MinShowtimeHandle = FTSTicker::GetCoreTicker().AddTicker(TEXT("USaveLoadMarkerViewModel::MinShowtime"), *ConfiguredMinShowtimeSeconds,
		[this](float)
		{
			bIsShowingForMinShowtime = false;
			if (SaveGameService.IsValid() && SaveGameService->GetCurrentStatus() == USaveGameService::EStatus::Idle)
			{
				UpdateForStatus(USaveGameService::EStatus::Idle);
			}
			return false; // = Don't restart timer.
		});
}
