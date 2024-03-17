///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
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
