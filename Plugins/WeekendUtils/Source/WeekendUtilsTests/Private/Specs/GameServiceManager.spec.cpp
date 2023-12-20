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
#include "GameService/GameServiceConfig.h"
#include "GameService/GameServiceManager.h"
#include "Mocks/GameServiceMocks.h"

#define SPEC_TEST_CATEGORY "WeekendUtils.GameService"

using namespace WeekendUtils;
using namespace Mocks;

WE_BEGIN_DEFINE_SPEC(GameServiceManager)
	TSharedPtr<FScopedAutomationTestWorld> TestWorld;
	TObjectPtr<UGameServiceManager> ServiceManager;
WE_END_DEFINE_SPEC(GameServiceManager)
{
	BeforeEach([this]
	{
		TestWorld = MakeShared<FScopedAutomationTestWorld>(SpecTestWorldName);
		TestWorld->InitializeGame();
		ServiceManager = UGameServiceManager::GetPtr();
		ServiceManager->ClearServiceRegister(); // Clear registrations from auto configs.
	});

	AfterEach([this]
	{
		ServiceManager = nullptr;
		TestWorld.Reset();
	});

	Describe("RegisterServices/RegisterServiceClass", [this]
	{
		// Also covers UGameServiceManager::IsServiceRegistered():
		It("should only register configured service classes", [this]
		{
			UGameServiceConfig* Config = NewObject<UGameServiceConfig>(TestWorld->AsPtr());
			Config->AddSingletonService<UVoidService, UVoidService2>();
			Config->AddSingletonService<UVoidObserverService>();
			Config->AddSingletonService<IMockGameServiceInterface, UInterfacedService>();
			TestEqual("RegisteredServicesClasses", ServiceManager->GetAllRegisteredServiceClasses().Num(), 0);

			ServiceManager->RegisterServices(*Config);
			TestEqual("RegisteredServiceClasses", ServiceManager->GetAllRegisteredServiceClasses().Num(), 3);

			TestTrue("IsServiceRegistered<UVoidService>()", ServiceManager->IsServiceRegistered<UVoidService>());
			TestTrue("IsServiceRegistered<UVoidObserverService>()", ServiceManager->IsServiceRegistered<UVoidObserverService>());
			TestTrue("IsServiceRegistered<IMockGameServiceInterface>()", ServiceManager->IsServiceRegistered<IMockGameServiceInterface>());

			TestFalse("IsServiceRegistered<UVoidService2>()", ServiceManager->IsServiceRegistered<UVoidService2>());
			TestFalse("IsServiceRegistered<UVoidObserverAssistantService>()", ServiceManager->IsServiceRegistered<UVoidObserverAssistantService>());
			TestFalse("IsServiceRegistered<UVoidObserverFanService>()", ServiceManager->IsServiceRegistered<UVoidObserverFanService>());
			TestFalse("IsServiceRegistered<UInterfacedService>()", ServiceManager->IsServiceRegistered<UInterfacedService>());
		});

		It("should only register the configured service instance classes", [this]
		{
			UGameServiceConfig* Config = NewObject<UGameServiceConfig>(TestWorld->AsPtr());
			Config->AddSingletonService<UVoidService, UVoidService2>();
			Config->AddSingletonService<UVoidObserverService>();
			Config->AddSingletonService<IMockGameServiceInterface, UInterfacedService>();
			TestEqual("RegisteredInstanceClasses", ServiceManager->GetAllRegisteredServiceInstanceClasses().Num(), 0);

			using FRegisteredInstanceClasses = GameService::TDependencyList<UGameServiceBase>;
			FRegisteredInstanceClasses RegisteredInstanceClasses;

			ServiceManager->RegisterServices(*Config);
			RegisteredInstanceClasses.Elements = ServiceManager->GetAllRegisteredServiceInstanceClasses();
			TestEqual("RegisteredInstanceClasses", RegisteredInstanceClasses.Num(), 3);

			TestTrue("UVoidService2 in RegisteredInstanceClasses", RegisteredInstanceClasses.Contains<UVoidService2>());
			TestTrue("UVoidObserverService in RegisteredInstanceClasses", RegisteredInstanceClasses.Contains<UVoidObserverService>());
			TestTrue("UInterfacedService in RegisteredInstanceClasses", RegisteredInstanceClasses.Contains<UInterfacedService>());

			TestFalse("UVoidService in RegisteredInstanceClasses", RegisteredInstanceClasses.Contains<UVoidService>());
			TestFalse("UVoidObserverAssistantService in RegisteredInstanceClasses", RegisteredInstanceClasses.Contains<UVoidObserverAssistantService>());
			TestFalse("UVoidObserverFanService in RegisteredInstanceClasses", RegisteredInstanceClasses.Contains<UVoidObserverFanService>());
		});

		It("should overwrite registrations with lower or equal priority", [this]
		{
			UGameServiceConfig* Config100 = NewObject<UGameServiceConfig>(TestWorld->AsPtr());
			Config100->AddSingletonService<UVoidService, UVoidService>();
			Config100->SetPriority(100);

			UGameServiceConfig* Config99 = NewObject<UGameServiceConfig>(TestWorld->AsPtr());
			Config99->AddSingletonService<UVoidService, UVoidService2>();
			Config99->AddSingletonService<UVoidObserverFanService>();
			Config99->SetPriority(99);

			UGameServiceConfig* Config101 = NewObject<UGameServiceConfig>(TestWorld->AsPtr());
			Config101->AddSingletonService<UVoidService, UVoidService2>();
			Config101->AddSingletonService<UVoidObserverService>();
			Config101->SetPriority(101);

			UGameServiceConfig* Config101_2 = NewObject<UGameServiceConfig>(TestWorld->AsPtr());
			Config101_2->AddSingletonService<UVoidService, UVoidService>();
			Config101_2->SetPriority(101);

			using FRegisteredInstanceClasses = GameService::TDependencyList<UGameServiceBase>;
			FRegisteredInstanceClasses RegisteredInstanceClasses;

			ServiceManager->RegisterServices(*Config100);
			RegisteredInstanceClasses.Elements = ServiceManager->GetAllRegisteredServiceInstanceClasses();
			TestTrue("UVoidService in RegisteredInstanceClasses", RegisteredInstanceClasses.Contains<UVoidService>());

			ServiceManager->RegisterServices(*Config99);
			RegisteredInstanceClasses.Elements = ServiceManager->GetAllRegisteredServiceInstanceClasses();
			TestTrue("UVoidService in RegisteredInstanceClasses", RegisteredInstanceClasses.Contains<UVoidService>());
			TestFalse("UVoidService2 in RegisteredInstanceClasses", RegisteredInstanceClasses.Contains<UVoidService2>());
			TestTrue("UVoidObserverFanService in RegisteredInstanceClasses", RegisteredInstanceClasses.Contains<UVoidObserverFanService>());

			ServiceManager->RegisterServices(*Config101);
			RegisteredInstanceClasses.Elements = ServiceManager->GetAllRegisteredServiceInstanceClasses();
			TestFalse("UVoidService in RegisteredInstanceClasses", RegisteredInstanceClasses.Contains<UVoidService>());
			TestTrue("UVoidService2 in RegisteredInstanceClasses", RegisteredInstanceClasses.Contains<UVoidService2>());
			TestTrue("UVoidObserverFanService in RegisteredInstanceClasses", RegisteredInstanceClasses.Contains<UVoidObserverFanService>());
			TestTrue("UVoidObserverService in RegisteredInstanceClasses", RegisteredInstanceClasses.Contains<UVoidObserverService>());

			AddExpectedError("has the same priority");
			ServiceManager->RegisterServices(*Config101_2);
			RegisteredInstanceClasses.Elements = ServiceManager->GetAllRegisteredServiceInstanceClasses();
			TestTrue("UVoidService in RegisteredInstanceClasses", RegisteredInstanceClasses.Contains<UVoidService>());
			TestFalse("UVoidService2 in RegisteredInstanceClasses", RegisteredInstanceClasses.Contains<UVoidService2>());
			TestTrue("UVoidObserverFanService in RegisteredInstanceClasses", RegisteredInstanceClasses.Contains<UVoidObserverFanService>());
			TestTrue("UVoidObserverService in RegisteredInstanceClasses", RegisteredInstanceClasses.Contains<UVoidObserverService>());
		});

		It("should NOT start any registered service", [this]
		{
			UGameServiceConfig* Config = NewObject<UGameServiceConfig>(TestWorld->AsPtr());
			Config->AddSingletonService<UVoidService, UVoidService2>();
			Config->AddSingletonService<UVoidObserverService>();
			Config->AddSingletonService<IMockGameServiceInterface, UInterfacedService>();

			ServiceManager->RegisterServices(*Config);
			TestFalse("WasServiceStarted<UVoidService>()", ServiceManager->WasServiceStarted<UVoidService>());
			TestFalse("WasServiceStarted<UVoidObserverService>()", ServiceManager->WasServiceStarted<UVoidObserverService>());
			TestFalse("WasServiceStarted<IMockGameServiceInterface>()", ServiceManager->WasServiceStarted<IMockGameServiceInterface>());
		});
	});

	Describe("StartService", [this]
	{
		// Also covers UGameServiceManager::FindStartedServiceInstance():
		It("should create a new service instance of the desired class", [this]
		{
			// Instance Class <- Service Class:
			ServiceManager->StartService<UVoidService>(TestWorld->AsRef());
			TestTrue("IsValid(VoidService)", IsValid(ServiceManager->FindStartedServiceInstance<UVoidService>()));

			// Instance Class = Service Class:
			ServiceManager->StartService<UVoidObserverService, UVoidObserverService>(TestWorld->AsRef());
			TestTrue("IsValid(VoidObserverService)", IsValid(ServiceManager->FindStartedServiceInstance<UVoidObserverService>()));

			// Instance Class != Service Class:
			ServiceManager->StartService<IMockGameServiceInterface, UInterfacedService>(TestWorld->AsRef());
			TestNotNull("IMockGameServiceInterface instance", ServiceManager->FindStartedServiceInstance<IMockGameServiceInterface, UInterfacedService>());
			TestNull("UInterfacedService instance", ServiceManager->FindStartedServiceInstance<UInterfacedService>());
		});

		It("should NOT start a new service instance for service instance classes registered under multiple service classes", [this]
		{
			uint8 NumOfStartCalls = 0;

			UVoidService2& Instance1 = ServiceManager->StartService<UVoidService, UVoidService2>(TestWorld->AsRef());
			NumOfStartCalls += Instance1.bWasStarted;
			Instance1.bWasStarted = false;

			UVoidService2& Instance2 = ServiceManager->StartService<UVoidService2, UVoidService2>(TestWorld->AsRef());
			NumOfStartCalls += Instance2.bWasStarted;

			TestTrue("IsServiceRegistered<UVoidService>()", ServiceManager->IsServiceRegistered<UVoidService>());
			TestTrue("IsServiceRegistered<UVoidService2>()", ServiceManager->IsServiceRegistered<UVoidService2>());
			TestSame("Created Instances", Instance1, Instance2);
			TestEqual("Number of StartService() calls", NumOfStartCalls, 1);
		});

		It("should start the created service instance and call StartService() on it", [this]
		{
			const UVoidService& Service = ServiceManager->StartService<UVoidService>(TestWorld->AsRef());
			TestTrue("ServiceManager->WasServiceStarted<UVoidService>()", ServiceManager->WasServiceStarted<UVoidService>());
			TestTrue("Service.bWasStarted", Service.bWasStarted);
			TestFalse("Service.bWasShutDown", Service.bWasShutDown);
		});

		It("should start the passed service instance and call StartService() on it", [this]
		{
			UVoidService& Service = *NewObject<UVoidService>(TestWorld->AsPtr());
			ServiceManager->StartService<UVoidService>(TestWorld->AsRef(), Service);
			TestTrue("ServiceManager->WasServiceStarted<UVoidService>()" , ServiceManager->WasServiceStarted<UVoidService>());
			TestTrue("Service.bWasStarted", Service.bWasStarted);
			TestFalse("Service.bWasShutDown", Service.bWasShutDown);
		});

		It("should NOT create a new service instance if one is already existing", [this]
		{
			UVoidService& ExistingService = *NewObject<UVoidService>(TestWorld->AsPtr());
			UVoidService& StartedService = ServiceManager->StartService<UVoidService>(TestWorld->AsRef(), ExistingService);
			TestSame("StartedService", StartedService, ExistingService);
		});

		// Also covers UGameServiceManager::WasServiceStarted():
		It("should start any dependent services before starting the requested service", [this]
		{
			// First register the dependency service classes, so the manager knows which classes to instance for the dependencies:
			UGameServiceConfig* Config = NewObject<UGameServiceConfig>(TestWorld->AsPtr());
			Config->AddSingletonService<UVoidService>();
			Config->AddSingletonService<UVoidObserverService>();
			ServiceManager->RegisterServices(*Config);

			// (i) Dependencies: UVoidObserverAssistantService -> UVoidObserverService -> UVoidService
			const UVoidObserverAssistantService& VoidObserverAssistantService = ServiceManager->StartService<UVoidObserverAssistantService>(TestWorld->AsRef());
			TestTrue("ServiceManager->WasServiceStarted<UVoidService>()", ServiceManager->WasServiceStarted<UVoidService>());
			TestTrue("ServiceManager->WasServiceStarted<UVoidObserverService>()", ServiceManager->WasServiceStarted<UVoidObserverService>());

			const UVoidService* VoidService = ServiceManager->FindStartedServiceInstance<UVoidService, UVoidService>();
			const UVoidObserverService* VoidObserverService = ServiceManager->FindStartedServiceInstance<UVoidObserverService, UVoidObserverService>();
			if (TestNotNull("VoidService", VoidService) && TestNotNull("VoidObserverService", VoidObserverService))
			{
				TestTrue("VoidService->WasStartedBefore(*VoidObserverService)",
				 VoidService->WasStartedBefore(*VoidObserverService));
				TestTrue("VoidService->WasStartedBefore(VoidObserverAssistantService)",
				 VoidService->WasStartedBefore(VoidObserverAssistantService));
				TestTrue("VoidObserverService->WasStartedBefore(VoidObserverAssistantService)",
				 VoidObserverService->WasStartedBefore(VoidObserverAssistantService));
			}
		});
	});

	Describe("StartRegisteredServices", [this]
	{
		It("should create instances for all previously registered service classes", [this]
		{
			UGameServiceConfig* Config = NewObject<UGameServiceConfig>(TestWorld->AsPtr());
			Config->AddSingletonService<UVoidService, UVoidService2>();
			Config->AddSingletonService<UVoidObserverService>();
			Config->AddSingletonService<IMockGameServiceInterface, UInterfacedService>();
			ServiceManager->RegisterServices(*Config);

			ServiceManager->StartRegisteredServices(TestWorld->AsRef());
			TestTrue("WasServiceStarted<UVoidService>()", ServiceManager->WasServiceStarted<UVoidService>());
			TestTrue("WasServiceStarted<UVoidObserverService>()", ServiceManager->WasServiceStarted<UVoidObserverService>());
			TestTrue("WasServiceStarted<IMockGameServiceInterface>()", ServiceManager->WasServiceStarted<IMockGameServiceInterface>());
		});
	});

	Describe("TryStartService", [this]
	{
		// Also covers UGameServiceManager::DetermineServiceInstanceClass():
		It("should NOT start the service if the service instance class cannot be determined", [this]
		{
			// This is only the case when passing an interface class as service class:
			const FGameServiceClass ServiceClass = IMockGameServiceInterface::UClassType::StaticClass();
			const TOptional<UGameServiceBase*> ServiceInstance = ServiceManager->TryStartService(TestWorld->AsRef(), ServiceClass);
			TestFalse("ServiceInstance.IsSet()", ServiceInstance.IsSet());
			TestFalse("WasServiceStarted<IMockGameServiceInterface>()", ServiceManager->WasServiceStarted<IMockGameServiceInterface>());
		});
	});

	Describe("ShutdownAllServices", [this]
	{
		It("should call ShutdownService() on all running service instances", [this]
		{
			const auto& VoidService = ServiceManager->StartService<UVoidService>(TestWorld->AsRef());
			const auto& VoidObserverService = ServiceManager->StartService<UVoidObserverService>(TestWorld->AsRef());
			const auto& VoidObserverFanService = ServiceManager->StartService<UVoidObserverFanService>(TestWorld->AsRef());

			ServiceManager->ShutdownAllServices();
			TestTrue("VoidService.bWasShutDown", VoidService.bWasShutDown);
			TestTrue("VoidObserverService.bWasShutDown", VoidObserverService.bWasShutDown);
			TestTrue("VoidObserverFanService.bWasShutDown", VoidObserverFanService.bWasShutDown);
		});

		It("should call ShutdownService() on all running service instances in their inverse starting order", [this]
		{
			const auto& VoidService = ServiceManager->StartService<UVoidService>(TestWorld->AsRef());
			const auto& VoidObserverService = ServiceManager->StartService<UVoidObserverService>(TestWorld->AsRef());
			const auto& VoidObserverFanService = ServiceManager->StartService<UVoidObserverFanService>(TestWorld->AsRef());

			ServiceManager->ShutdownAllServices();
			TestTrue("VoidService.WasShutdownAfter(VoidObserverService)",
				VoidService.WasShutdownAfter(VoidObserverService));
			TestTrue("VoidService.WasShutdownAfter(VoidObserverFanService)",
				VoidService.WasShutdownAfter(VoidObserverFanService));
			TestTrue("VoidObserverService.WasShutdownAfter(VoidObserverFanService)",
				VoidObserverService.WasShutdownAfter(VoidObserverFanService));
		});

		It("should stop tracking all services as running", [this]
		{
			ServiceManager->StartService<UVoidService>(TestWorld->AsRef());
			ServiceManager->StartService<UVoidObserverService>(TestWorld->AsRef());
			ServiceManager->StartService<UVoidObserverFanService>(TestWorld->AsRef());

			ServiceManager->ShutdownAllServices();
			TestFalse("WasServiceStarted<UVoidService>()", ServiceManager->WasServiceStarted<UVoidService>());
			TestFalse("WasServiceStarted<UVoidObserverService>()", ServiceManager->WasServiceStarted<UVoidObserverService>());
			TestFalse("WasServiceStarted<UVoidObserverFanService>()", ServiceManager->WasServiceStarted<UVoidObserverFanService>());
		});
	});

	Describe("ClearServiceRegister", [this]
	{
		It("should NOT stop any services", [this]
		{
			ServiceManager->StartService<UVoidService>(TestWorld->AsRef());
			ServiceManager->ClearServiceRegister();
			TestTrue("WasServiceStarted<UVoidService>()", ServiceManager->WasServiceStarted<UVoidService>());
		});

		It("should clear any previously registered service classes", [this]
		{
			UGameServiceConfig* Config = NewObject<UGameServiceConfig>(TestWorld->AsPtr());
			Config->AddSingletonService<UVoidService, UVoidService2>();
			Config->AddSingletonService<UVoidObserverService>();
			Config->AddSingletonService<IMockGameServiceInterface, UInterfacedService>();
			ServiceManager->RegisterServices(*Config);

			ServiceManager->ClearServiceRegister();
			TestTrue("No services were registered", ServiceManager->GetAllRegisteredServiceClasses().IsEmpty());
		});
	});
}

#undef SPEC_TEST_CATEGORY
#endif WITH_AUTOMATION_WORKER
