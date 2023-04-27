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

UGameServiceConfig& UGameServiceConfig::CreateForWorld(UWorld& World, uint32 Priority, TFunction<void(UGameServiceConfig&)> ConfigExec)
{
	UGameServiceConfig* NewConfig = NewObject<UGameServiceConfig>(&World);
	NewConfig->RegisterPriority = Priority;
	ConfigExec(*NewConfig);
	NewConfig->RegisterWithGameServiceManager();
	return *NewConfig;
}

void UGameServiceConfig::RegisterWithGameServiceManager() const
{
	UE_LOG(LogGameService, Verbose, TEXT("Registering GameServiceConfig (%s) with GameServiceManager"), *GetName());
	UGameServiceManager::Get().RegisterServices(*this);
}
