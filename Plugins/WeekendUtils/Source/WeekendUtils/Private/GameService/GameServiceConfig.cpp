///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz in cooperation with Nine Worlds Studios GmbH.
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See accompanying file LICENSE.
///
///////////////////////////////////////////////////////////////////////////////////////

#include "GameService/GameServiceConfig.h"

#include "GameService/GameServiceManager.h"
#include "WeekendUtils.h"
#include "Initialization/WorldGameServiceRunner.h"

UGameServiceConfig& UGameServiceConfig::CreateForWorld(UWorld& World, TFunction<void(UGameServiceConfig&)> ConfigExec)
{
	UGameServiceConfig* NewConfig = NewObject<UGameServiceConfig>(&World);
	ConfigExec(*NewConfig);
	NewConfig->RegisterWithGameServiceManager();
	return *NewConfig;
}

UGameServiceConfig& UGameServiceConfig::CreateForNextWorld(TFunction<void(UGameServiceConfig&)> ConfigExec)
{
	UGameServiceConfig* NewConfig = NewObject<UGameServiceConfig>(GetTransientPackage());
	ConfigExec(*NewConfig);
	UWorldGameServiceRunner::SetServiceConfigForNextWorld(*NewConfig);
	return *NewConfig;
}

void UGameServiceConfig::RegisterWithGameServiceManager() const
{
	UE_LOG(LogGameService, Verbose, TEXT("Registering GameServiceConfig (%s) with GameServiceManager"), *GetName());
	UGameServiceManager::Get().RegisterServices(*this);
}
