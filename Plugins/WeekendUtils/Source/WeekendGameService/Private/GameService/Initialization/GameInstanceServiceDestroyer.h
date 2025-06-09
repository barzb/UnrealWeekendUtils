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

#include "GameInstanceServiceDestroyer.generated.h"

class UGameServiceConfig;

/**
 * The singleton instance of this subsystem for each game instance will take care of maintaining the
 * game services whose lifetime is bound to it, in cooperation with the persistent @UGameServiceManager.
 *
 * The runner will do the following things:
 * - Shutdown relevant running services when the game instance shuts down
 * - Clears relevant registered service configs when the game instance shuts down
 */
UCLASS()
class UGameInstanceServiceDestroyer : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// - UGameInstanceSubsystem
	virtual void Deinitialize() override;
	// --
};

