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
	DECLARE_DELEGATE(FOnWaitingFinished)
	using FGameServiceUser::WaitForDependencies;
	using FGameServiceUser::UseGameService;
	using FGameServiceUser::UseGameServiceAsPtr;
	using FGameServiceUser::UseGameServiceAsInterface;
	using FGameServiceUser::FindOptionalGameService;
	using FGameServiceUser::FindSubsystemDependency;
	using FGameServiceUser::StopWaitingForDependencies;
	using FGameServiceUser::InvalidateCachedDependencies;

	void SimulateTick()
	{
		PollPendingDependencyWaitCallbacks();
	}

	// - FGameServiceUser
	virtual FGameServiceUserConfig ConfigureGameServiceUser() const override
	{
		// This mock allows configurations to change, so the cache must be updated:
		InvalidateCachedDependencies();

		FGameServiceUserConfig Config(this);
		Config.ServiceDependencies = ServiceDependencies;
		Config.SubsystemDependencies = SubsystemDependencies;
		Config.OptionalSubsystemDependencies = OptionalSubsystemDependencies;
		return Config;
	}
	// --

	FGameServiceDependencies ServiceDependencies;
	FSubsystemDependencies SubsystemDependencies;
	FSubsystemDependencies OptionalSubsystemDependencies;
};
