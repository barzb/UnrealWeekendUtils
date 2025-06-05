///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#if WITH_AUTOMATION_WORKER

#include "GameService/GameServiceUtils.h"
#include "GameService/Mocks/GameServiceMocks.h"

using namespace Mocks;

namespace GameService::Tests
{
	/**
	 * The method does not need to be called anywhere.
	 * Compile-time tests, not executed by the test runner, but they will throw compile errors when the templates are broken.
	 */
	struct FGameServiceUtilsTemplates
	{
		FGameServiceUtilsTemplates()
		{
			TDependencyList<UGameServiceBase> ServiceTypeDependencies;
			TDependencyList<USubsystem> SubsystemTypeDependencies;

			//GameService::TDependencyList.Add<T>
			ServiceTypeDependencies.Add<UVoidService>();
			ServiceTypeDependencies.Add<UVoidObserverService>();
			ServiceTypeDependencies.Add<IMockGameServiceInterface>();
			ServiceTypeDependencies.Add<UWorldSubsystem>();
			ServiceTypeDependencies.Add<UEngineSubsystem>();
			ServiceTypeDependencies.Add<UGameInstanceSubsystem>();
			ServiceTypeDependencies.Add<ULocalPlayerSubsystem>();

			// GameService::TDependencyList.Contains<T>
			(bool) ServiceTypeDependencies.Contains<UVoidService>();
			(bool) ServiceTypeDependencies.Contains<UVoidObserverService>();
			(bool) ServiceTypeDependencies.Contains<IMockGameServiceInterface>();
			(bool) ServiceTypeDependencies.Contains<UWorldSubsystem>();
			(bool) ServiceTypeDependencies.Contains<UEngineSubsystem>();
			(bool) ServiceTypeDependencies.Contains<UGameInstanceSubsystem>();
			(bool) ServiceTypeDependencies.Contains<ULocalPlayerSubsystem>();
		}
	} GInstance;
}

#endif WITH_AUTOMATION_WORKER