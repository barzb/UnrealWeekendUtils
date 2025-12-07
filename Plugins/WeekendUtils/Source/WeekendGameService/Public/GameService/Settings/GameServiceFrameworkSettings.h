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
#include "Engine/DeveloperSettings.h"

#include "GameServiceFrameworkSettings.generated.h"

class UGameServiceConfig;

/**
 * Project settings for the Game Service Framework and its surrounding API.
 */
UCLASS(Config = Game, DefaultConfig, DisplayName = "Game Services")
class WEEKENDGAMESERVICE_API UGameServiceFrameworkSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	void RegisterServiceConfig(UGameServiceConfig& Config);

	/** Configurable class default objects of all auto-registered Game Service Configs. */
	UPROPERTY(VisibleAnywhere, NoClear, EditFixedSize, meta = (EditInline), Category = "Weekend Utils|Game Service")
	TMap<FName, TObjectPtr<UGameServiceConfig>> GameServiceConfigs = {};
};
