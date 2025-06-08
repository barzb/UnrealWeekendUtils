///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#if WITH_AUTOMATION_WORKER

#include "AutomationTest/AutomationSpecMacros.h"
#include "AutomationTest/AutomationTestWorld.h"
#include "GameService/GameServiceManager.h"
#include "GameService/Mocks/GameServiceMocks.h"

#define SPEC_TEST_CATEGORY "WeekendUtils.GameService"

using namespace WeekendUtils;
using namespace Mocks;

/**
 * @UGameInstanceServiceDestroyer is not accessible outside the WeekendUtils module,
 * but some of its inner workings can still be verified via tests.
 */
WE_BEGIN_DEFINE_SPEC(GameInstanceServiceDestroyer)
	TSharedPtr<FScopedAutomationTestWorld> TestWorld;
	TObjectPtr<UGameInstanceSubsystem> GameInstanceServiceDestroyer;
WE_END_DEFINE_SPEC(GameInstanceServiceDestroyer)
{
	BeforeEach([this]
	{
		TestWorld = MakeShared<FScopedAutomationTestWorld>(SpecTestWorldName);
		TestWorld->InitializeGame();

		// This is a bit hacky, but because the UGameInstanceServiceDestroyer type info is private in its source module, we'll find it by name:
		TArray<UGameInstanceSubsystem*> GameInstanceSubsystems = TestWorld->GameInstance->GetSubsystemArrayCopy<UGameInstanceSubsystem>();
		auto** Finder = GameInstanceSubsystems.FindByPredicate([](UGameInstanceSubsystem* Subsystem) {
			return (Subsystem->GetName().Contains("GameInstanceServiceDestroyer"));
		});
		GameInstanceServiceDestroyer = (ensure(Finder != nullptr) ? *Finder : nullptr);
	});

	AfterEach([this]
	{
		TestWorld.Reset();
	});

	Describe("Deinitialize", [this]
	{
		It("should shutdown all running GameServices with EGameServiceLifetime::ShutdownWithGameInstance.", [this]
		{
			UGameServiceManager& ServiceManager = UGameServiceManager::Get();
			UVoidService& GameInstanceService  = *NewObject<UVoidService>(TestWorld->AsPtr());
			GameInstanceService.Lifetime = EGameServiceLifetime::ShutdownWithGameInstance;
			ServiceManager.StartService<UVoidService>(TestWorld->AsRef(), GameInstanceService);

			GameInstanceServiceDestroyer->Deinitialize();
			TestTrue("GameInstanceService.bWasShutDown ", GameInstanceService.bWasShutDown);
		});

		It("should shutdown all running GameServices with EGameServiceLifetime::ShutdownWithWorld.", [this]
		{
			UGameServiceManager& ServiceManager = UGameServiceManager::Get();
			UVoidService& WorldService  = *NewObject<UVoidService>(TestWorld->AsPtr());
			WorldService.Lifetime = EGameServiceLifetime::ShutdownWithWorld;
			ServiceManager.StartService<UVoidService>(TestWorld->AsRef(), WorldService);

			GameInstanceServiceDestroyer->Deinitialize();
			TestTrue("WorldService.bWasShutDown ", WorldService.bWasShutDown);
		});
	});
}

#undef SPEC_TEST_CATEGORY
#endif WITH_AUTOMATION_WORKER
