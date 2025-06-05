///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "GameService/Settings/GameServiceFrameworkSettings.h"

#include "GameService/GameServiceConfig.h"

void UGameServiceFrameworkSettings::RegisterServiceConfig(UGameServiceConfig& Config)
{
	GameServiceConfigs.Add(Config.GetClass()->GetFName(), &Config);
}
