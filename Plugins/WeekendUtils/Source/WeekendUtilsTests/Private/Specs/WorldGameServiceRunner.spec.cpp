///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz and contributors. See file: CREDITS.md
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
#include "Mocks/GameServiceMocks.h"
#include "Mocks/GameServiceUserMocks.h"

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
		TArray<UTickableWorldSubsystem*> WorldSubsystems = TestWorld->AsRef().GetSubsystemArray<UTickableWorldSubsystem>();
		auto** Finder = WorldSubsystems.FindByPredicate([](UTickableWorldSubsystem* Subsystem) {
			return (Subsystem->GetName().Contains("WorldGameServiceRunner"));
		});
		WorldGameServiceRunner = (ensure(Finder != nullptr) ? *Finder : nullptr);
	});

	AfterEach([this]
	{
		TestWorld.Reset();
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
		});
	});
}

#undef SPEC_TEST_CATEGORY
#endif WITH_AUTOMATION_WORKER
