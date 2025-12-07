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
#include "Subsystems/GameInstanceSubsystem.h"

#include "GameInstanceServiceTerminator.generated.h"

class UGameServiceConfig;

/**
 * The singleton instance of this subsystem for each game instance will take care of terminating the
 * @UGameServiceManager and all its services whose lifetime bound to the game instance.
 *
 * The runner will do the following things:
 * - Shutdown ALL running services when the game instance shuts down
 * - Clears ALL registered service configs when the game instance shuts down
 * - Terminate the UGameServiceManager instance for this GameInstance
 */
UCLASS(MinimalAPI)
class UGameInstanceServiceTerminator final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// - UGameInstanceSubsystem
	virtual void Deinitialize() override;
	// --
};

