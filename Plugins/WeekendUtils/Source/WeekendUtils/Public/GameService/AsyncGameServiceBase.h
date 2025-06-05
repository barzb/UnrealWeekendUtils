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
#include "GameService/GameServiceBase.h"

#include "AsyncGameServiceBase.generated.h"

DECLARE_DELEGATE(FOnAsyncGameServiceStarted)

/**
 * Extension base class of @UGameServiceBase that provides additional utilities and interface to start and shutdown
 * a derived service in a deferred way.
 * This class already offers the following functionalities for async starting:
 * -> @bWaitForDependenciesBeforeStarting - Will start the service only once all non-optional dependencies are ready.
 * -> @bWaitForWorldToBeginPlay - Will start the service only once the current world has begun play.
 */
UCLASS(Abstract)
class WEEKENDUTILS_API UAsyncGameServiceBase : public UGameServiceBase
{
	GENERATED_BODY()

public:
	// - UGameServiceBase
	virtual void StartService() override final;
	virtual bool IsTickable() const override { return IsServiceRunning(); }
	virtual void ShutdownService() override;
	virtual TOptional<FString> GetServiceStatusInfo() const override;
	// --

	/** @returns whether the service has finished starting and is currently considered 'running'. */
	virtual bool IsServiceRunning() const { return (CurrentStatus == EAsyncServiceStatus::Running); }

	/** Calls provided callback right after this service is fully started and running - or immediately if already running. */
	void WaitUntilServiceIsRunning(FOnAsyncGameServiceStarted Callback);

	template <typename CallbackType>
	void WaitUntilServiceIsRunning(UObject& Caller, CallbackType Callback)
	{
		WaitUntilServiceIsRunning(FOnAsyncGameServiceStarted::CreateWeakLambda(&Caller, Callback));
	}

	/**
	 * Called when the service starts to kick off the deferred starting process.
	 * Derived classes must manually call @FinishServiceStart() when they are fully started.
	 * when the service is considered 'running'.
	 */
	virtual void BeginServiceStart() PURE_VIRTUAL(BeginServiceStart);
	void FinishServiceStart();

	/**
	 * Called when the service starts to kick off the deferred shutdown process.
	 * Derived classes must manually call @FinishServiceShutdown() when they are fully shut down.
	 * Be aware that any service dependencies are only promised to be valid in the BeginServiceShutdown() call.
	 * The service object will be artificially kept alive until it is fully shutdown, except for when
	 * @bIsWorldTearingDown is true, then the service should shutdown immediately (if possible).
	 */
	virtual void BeginServiceShutdown(bool bIsWorldTearingDown) PURE_VIRTUAL(BeginServiceShutdown);
	void FinishServiceShutdown();

protected:
	enum class EAsyncServiceStatus : uint8
	{
		Inactive = 0,
		Starting = 1,
		Running = 2,
		Stopping = 3,
	} CurrentStatus = EAsyncServiceStatus::Inactive;

	/** When enabled by derived class, the @BeginServiceStart() call will be deferred until all configured dependencies are available. */
	bool bWaitForDependenciesBeforeStarting = true;

	/** When enabled by derived class, the @BeginServiceStart() call will be deferred until the world has begun play. */
	bool bWaitForWorldToBeginPlay = false;

private:
	TArray<FOnAsyncGameServiceStarted> PendingServiceStartCallbacks;

	bool bIsWaitingForDependencies = false;
	bool bIsWaitingForWorldToBeginPlay = false;
	FDelegateHandle WaitForWorldDelegateHandle;

	void AttemptToStartService();

	void WaitForWorldToBeginPlay(UWorld& World);
};
