///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz in cooperation with Nine Worlds Studios GmbH.
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See accompanying file LICENSE.
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameService/GameServiceConfig.h"

#include "WorldServiceConfigBase.generated.h"

/**
 * Base class for configurations whose CDO will be automatically registered with the @UGameServiceManager.
 * Derived configurations should define at least one world name pattern.
 * When a world whith matching name is entered, the @UWorldGameServiceRunner will automatically register
 * the CDO of the config with the service manager.
 */
UCLASS(Abstract)
class WEEKENDUTILS_API UWorldServiceConfigBase : public UGameServiceConfig
{
	GENERATED_BODY()

public:
	/** @returns the CDO of the world service config class configured for given world, or nullptr. */
	static const UWorldServiceConfigBase* FindConfigForWorld(const UWorld& World);

protected:
	/** Registers this config class to be used for worlds whose name contains given string (case-insensitive). */
	void RegisterForWorldsWhoseNamesContain(const FString& PartOfWorldName);
};

