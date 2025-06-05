///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "GameService/Mocks/GameServiceMocks.h"

void UMockGameServiceBase::StartService()
{
	ensureMsgf(IsValid(GetWorld()), TEXT("MockGameService was created without 'Outer', which is not supported."));
	static uint64 StartIndexEnumerator = 0;
	static TWeakObjectPtr<UWorld> EnumeratorWorld = nullptr;
	if (!EnumeratorWorld.IsValid())
	{
		// Reset enumerator for new worlds, so each test world starts back at 0:
		StartIndexEnumerator = 0;
		EnumeratorWorld = MakeWeakObjectPtr(GetWorld());
	}

	StartIndex = StartIndexEnumerator++;
	bWasStarted = true;
}

void UMockGameServiceBase::TickService(float DeltaTime)
{
	TickCounter++;
}

void UMockGameServiceBase::ShutdownService()
{
	ensureMsgf(IsValid(GetWorld()), TEXT("MockGameService was created without 'Outer', which is not supported."));
	static uint64 ShutdownIndexEnumerator = 0;
	static TWeakObjectPtr<UWorld> EnumeratorWorld = nullptr;
	if (!EnumeratorWorld.IsValid())
	{
		// Reset enumerator for new worlds, so each test world starts back at 0:
		ShutdownIndexEnumerator = 0;
		EnumeratorWorld = MakeWeakObjectPtr(GetWorld());
	}

	ShutdownIndex = ShutdownIndexEnumerator++;
	bWasStarted = false;
	bWasShutDown = true;
}

bool UMockGameServiceBase::WasStartedBefore(const UMockGameServiceBase& OtherService) const
{
	return (StartIndex < OtherService.StartIndex);
}

bool UMockGameServiceBase::WasStartedAfter(const UMockGameServiceBase& OtherService) const
{
	return (StartIndex > OtherService.StartIndex);
}

bool UMockGameServiceBase::WasShutdownBefore(const UMockGameServiceBase& OtherService) const
{
	return (ShutdownIndex < OtherService.ShutdownIndex);
}

bool UMockGameServiceBase::WasShutdownAfter(const UMockGameServiceBase& OtherService) const
{
	return (ShutdownIndex > OtherService.ShutdownIndex);
}
