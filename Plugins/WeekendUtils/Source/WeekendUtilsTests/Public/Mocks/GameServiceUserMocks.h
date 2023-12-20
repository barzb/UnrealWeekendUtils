///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameService/GameServiceUser.h"

#include "GameServiceUserMocks.generated.h"

/**
 * UObject wrapper for FGameServiceUser. The mock exposes some hidden functionalities to public
 * so tests can use the functionalities like a proper UObject would when deriving from it.
 */
UCLASS(Hidden, NotBlueprintable, NotBlueprintType)
class WEEKENDUTILSTESTS_API UGameServiceUserMock : public UObject, public FGameServiceUser
{
	GENERATED_BODY()

public:
	using FGameServiceUser::ServiceDependencies;
	using FGameServiceUser::SubsystemDependencies;

	DECLARE_DELEGATE(FOnWaitingFinished)
	using FGameServiceUser::WaitForDependencies;
	using FGameServiceUser::UseGameService;
	using FGameServiceUser::UseGameServiceAsPtr;
	using FGameServiceUser::FindOptionalGameService;
	using FGameServiceUser::FindSubsystemDependency;
	using FGameServiceUser::StopWaitingForDependencies;

	void SimulateTick()
	{
		PollPendingDependencyWaitCallbacks(this);
	}
};
