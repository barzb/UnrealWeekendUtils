///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz in cooperation with Nine Worlds Studios GmbH.
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See accompanying file LICENSE.
///
///////////////////////////////////////////////////////////////////////////////////////

#include "GameService/AsyncGameServiceBase.h"

void UAsyncGameServiceBase::StartService()
{
	ensure(CurrentStatus == EAsyncServiceStatus::Inactive);
	CurrentStatus = EAsyncServiceStatus::Starting;

	if (!bWaitForDependenciesBeforeStarting || AreAllDependenciesReady(this))
	{
		BeginServiceStart();
	}
	else
	{
		WaitForDependencies(this, FOnWaitingFinished::CreateUObject(this, &UAsyncGameServiceBase::BeginServiceStart));
	}
}

void UAsyncGameServiceBase::ShutdownService()
{
	StopWaitingForDependencies(this);

	const bool bIsWorldTearingDown = (!IsValid(GetWorld()) && GetWorld()->bIsTearingDown);
	if (!bIsWorldTearingDown)
	{
		AddToRoot(); // Make sure service is not garbage collected while shutting down.
	}

	CurrentStatus = EAsyncServiceStatus::Stopping;
	BeginServiceShutdown(bIsWorldTearingDown);
}

void UAsyncGameServiceBase::FinishServiceStart()
{
	CurrentStatus = EAsyncServiceStatus::Running;

	while (PendingServiceStartCallbacks.Num() > 0)
	{
		PendingServiceStartCallbacks.Pop().ExecuteIfBound();
	}
}

void UAsyncGameServiceBase::FinishServiceShutdown()
{
	CurrentStatus = EAsyncServiceStatus::Inactive;
	RemoveFromRoot(); // Free up for GC again.
}

void UAsyncGameServiceBase::WaitUntilServiceIsRunning(FOnAsyncGameServiceStarted Callback)
{
	if (IsServiceRunning())
	{
		Callback.ExecuteIfBound();
	}
	else
	{
		PendingServiceStartCallbacks.Add(Callback);
	}
}
