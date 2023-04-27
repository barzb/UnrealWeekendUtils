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
#include "GameService/GameServiceBase.h"

#include "GameServiceMocks.generated.h"

//////////////////////////////////////////////////////////////////////
/// (i) See bottom of this file for all type-aliased mock classes. ///
//////////////////////////////////////////////////////////////////////

/**
 * Base class for GameService mocks. Provides meta information about derived member function calls.
 * @requires child classes to be instanced with a valid outer for GetWorld() access.
 */
UCLASS(Abstract, Hidden, NotBlueprintable, NotBlueprintType, ClassGroup=Tests)
class WEEKENDUTILSTESTS_API UMockGameServiceBase : public UGameServiceBase
{
	GENERATED_BODY()

public:
	bool bWasStarted = false;
	bool bIsTickable = false;
	bool bWasShutDown = false;
	uint16 TickCounter = 0;

	// - UGameServiceBase
	virtual void StartService() override;
	virtual bool IsTickable() const override { return bIsTickable; }
	virtual void TickService(float DeltaTime) override;
	virtual void ShutdownService() override;
	// --

	bool WasStartedBefore(const UMockGameServiceBase& OtherService) const;
	bool WasStartedAfter(const UMockGameServiceBase& OtherService) const;

	bool WasShutdownBefore(const UMockGameServiceBase& OtherService) const;
	bool WasShutdownAfter(const UMockGameServiceBase& OtherService) const;

private:
	uint64 StartIndex = 0; // See: WasStartedBefore/After()
	uint64 ShutdownIndex = 0; // See: WasShutdownBefore/After()
};

//////////////////////////////////////////////////////////////////////

UINTERFACE(NotBlueprintable)
class WEEKENDUTILSTESTS_API UMockGameServiceInterface : public UInterface { GENERATED_BODY() };
class WEEKENDUTILSTESTS_API IMockGameServiceInterface { GENERATED_BODY() };

UCLASS(Hidden, ClassGroup=Tests)
class WEEKENDUTILSTESTS_API UMockGameService_Interfaced : public UMockGameServiceBase, public IMockGameServiceInterface { GENERATED_BODY() };

//////////////////////////////////////////////////////////////////////

UCLASS(Hidden, ClassGroup=Tests)
class WEEKENDUTILSTESTS_API UMockGameService_Void : public UMockGameServiceBase { GENERATED_BODY() };

UCLASS(Hidden, ClassGroup=Tests)
class WEEKENDUTILSTESTS_API UMockGameService_Void2 : public UMockGameService_Void { GENERATED_BODY() };

UCLASS(Hidden, ClassGroup=Tests)
class WEEKENDUTILSTESTS_API UMockGameService_VoidObserver : public UMockGameServiceBase
{
	GENERATED_BODY()

public:
	UMockGameService_VoidObserver()
	{
		ServiceDependencies.Add<UMockGameService_Void>();
	}
};

UCLASS(Hidden, ClassGroup=Tests)
class WEEKENDUTILSTESTS_API UMockGameService_VoidObserverAssistant : public UMockGameServiceBase
{
	GENERATED_BODY()

public:
	UMockGameService_VoidObserverAssistant()
	{
		ServiceDependencies.Add<UMockGameService_VoidObserver>();
	}
};

UCLASS(Hidden, ClassGroup=Tests)
class WEEKENDUTILSTESTS_API UMockGameService_VoidObserverFan : public UMockGameServiceBase
{
	GENERATED_BODY()

public:
	UMockGameService_VoidObserverFan()
	{
		ServiceDependencies.Add<UMockGameService_VoidObserver>();
	}
};

//////////////////////////////////////////////////////////////////////

namespace Mocks
{
	using UInterfacedService = UMockGameService_Interfaced;
	using UVoidService = UMockGameService_Void;
	using UVoidService2 = UMockGameService_Void2;
	using UVoidObserverService = UMockGameService_VoidObserver;
	using UVoidObserverAssistantService = UMockGameService_VoidObserverAssistant;
	using UVoidObserverFanService = UMockGameService_VoidObserverFan;
}