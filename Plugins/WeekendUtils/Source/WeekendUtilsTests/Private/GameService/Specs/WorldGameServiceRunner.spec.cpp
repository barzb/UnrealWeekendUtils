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
 * @UWorldGameServiceRunner is not accessible outside the WeekendUtils module,
 * but some of its inner workings can still be verified via tests.
 */
WE_BEGIN_DEFINE_SPEC(WorldGameServiceRunner)
	TSharedPtr<FScopedAutomationTestWorld> TestWorld;
	TObjectPtr<UTickableWorldSubsystem> WorldGameServiceRunner;
	void TickWorldGameServiceRunner() const
	{
		if (IsValid(WorldGameServiceRunner))
		{ 
			WorldGameServiceRunner->Tick(0.f);
		}
	}
WE_END_DEFINE_SPEC(WorldGameServiceRunner)
{
	BeforeEach([this]
	{
		TestWorld = MakeShared<FScopedAutomationTestWorld>(SpecTestWorldName);
		TestWorld->InitializeGame();

		// This is a bit hacky, but because the UWorldGameServiceRunner type info is private in its source module, we'll find it by name:
		TArray<UTickableWorldSubsystem*> WorldSubsystems = TestWorld->AsRef().GetSubsystemArrayCopy<UTickableWorldSubsystem>();
		auto** Finder = WorldSubsystems.FindByPredicate([](UTickableWorldSubsystem* Subsystem) {
			return (Subsystem->GetName().Contains("WorldGameServiceRunner"));
		});
		WorldGameServiceRunner = (ensure(Finder != nullptr) ? *Finder : nullptr);
	});

	AfterEach([this]
	{
	});

	Describe("TickRunningServices", [this]
	{
		It("should call TickService() on all running services that are tickable.", [this]
		{
			UGameServiceManager& ServiceManager = UGameServiceManager::Get();
			const UVoidService& NonTickableService = ServiceManager.StartService<UVoidService>(TestWorld->AsRef());
			UVoidObserverService& TickableService  = ServiceManager.StartService<UVoidObserverService>(TestWorld->AsRef());
			TickableService.bIsTickable = true;
			TestEqual("NonTickableService.TickCounter ", NonTickableService.TickCounter, 0);
			TestEqual("TickableService.TickCounter ", TickableService.TickCounter, 0);

			TickWorldGameServiceRunner();
			TestEqual("NonTickableService.TickCounter", NonTickableService.TickCounter, 0);
			TestEqual("TickableService.TickCounter", TickableService.TickCounter, 1);

			TickWorldGameServiceRunner();
			TestEqual("NonTickableService.TickCounter", NonTickableService.TickCounter, 0);
			TestEqual("TickableService.TickCounter", TickableService.TickCounter, 2);

			TestWorld.Reset();
		});
	});

	Describe("Deinitialize", [this]
	{
		It("should shutdown all running GameServices with EGameServiceLifetime::ShutdownWithWorld.", [this]
		{
			UGameServiceManager& ServiceManager = UGameServiceManager::Get();
			UVoidService& WorldService  = *NewObject<UVoidService>(TestWorld->GameInstance);
			WorldService.Lifetime = EGameServiceLifetime::ShutdownWithWorld;
			ServiceManager.StartService<UVoidService>(TestWorld->AsRef(), WorldService);

			TestWorld.Reset(); // Destroys world and calls Deinitialize on subsystem.
			TestTrue("WorldService.bWasShutDown ", WorldService.bWasShutDown);
			WorldService.MarkAsGarbage();
		});

		It("should NOT shutdown any running GameServices with EGameServiceLifetime::ShutdownWithGameInstance.", [this]
		{
			UGameServiceManager& ServiceManager = UGameServiceManager::Get();
			UVoidService& GameInstanceService  = *NewObject<UVoidService>(TestWorld->GameInstance);
			GameInstanceService.Lifetime = EGameServiceLifetime::ShutdownWithGameInstance;
			ServiceManager.StartService<UVoidService>(TestWorld->AsRef(), GameInstanceService);

			TestWorld->World->CleanupWorld(); // Calls Deinitialize on world subsystem, but not GameInstance subsystems.
			TestFalse("GameInstanceService.bWasShutDown ", GameInstanceService.bWasShutDown);
			AddExpectedError("CleanupWorld called twice");
			TestWorld.Reset();
			GameInstanceService.MarkAsGarbage();
		});
	});
}

#undef SPEC_TEST_CATEGORY
#endif WITH_AUTOMATION_WORKER
