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
#include "GameService/GameServiceUser.h"
#include "UObject/Object.h"

#include "GameServiceBase.generated.h"

/**
 * Base class for all game services.
 * Provides an interface for @UGameServiceManager to start, stop and (potentially) tick the service.
 * Each derived service class also gains access to the inherited GameServiceUser API to configure
 * and access dependencies to other services or subsystems.
 */
UCLASS(Abstract)
class WEEKENDUTILS_API UGameServiceBase : public UObject, public FGameServiceUser
{
	GENERATED_BODY()

public:
	/**
	 * Called when the service is started. All configured service dependencies are promised to already run at this point.
	 * Services can be accessed via @UseService<>() or @FindOptionalService<>(), but subsystem dependencies might not yet
	 * be available. Consider using @WaitForDependencies() in this case.
	 */
	virtual void StartService() {}

	/** @returns whether this service should tick or not. If so, the TickService() method will be called automatically. */
	virtual bool IsTickable() const { return false; }

	/** Called each tick after the service has been started, when IsTickable() returns true. */
	virtual void TickService(float DeltaTime) {}

	/**
	 * Called before the service is destroyed, to provide an opportunity to stop function and clean up.
	 * All configured service dependencies are promised to be still alive and running when this is called.
	 */
	virtual void ShutdownService() {}

protected:
	/**
	 * Marks this class to be replicated from the server to clients.
	 * Supposed to be set in the constructor.
	 * #todo-multiplayer CURRENTLY NOT SUPPORTED
	 */
	bool bReplicates = false;

	/** @returns whether this service instance has network authority (= server). #todo-multiplayer CURRENTLY NOT SUPPORTED */
	bool HasAuthority() const { return true; }
};

using FGameServiceClass = TSubclassOf<UObject>;
using FGameServiceInstanceClass = TSubclassOf<UGameServiceBase>;
