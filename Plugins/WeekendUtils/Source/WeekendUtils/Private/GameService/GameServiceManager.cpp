///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "GameService/GameServiceManager.h"

#include "GameService/GameServiceConfig.h"
#include "WeekendUtils.h"
#include "GameService/AsyncGameServiceBase.h"

UGameServiceManager::UGameServiceManager()
{
	ServiceClassRegisters.Add(EGameServiceLifetime::ShutdownWithGameInstance);
	ServiceClassRegisters.Add(EGameServiceLifetime::ShutdownWithWorld);
}

void UGameServiceManager::RegisterServices(const UGameServiceConfig& Config)
{
	int32 NumRegistrations = 0;
	for (const auto Itr : Config.GetConfiguredServices())
	{
		const FGameServiceClass& RegisterClass = Itr.Key;
		const FGameServiceInstanceClass& InstanceClass = Itr.Value;
		const UGameServiceBase* TemplateInstance = Config.GetConfiguredServiceTemplate(RegisterClass);
		NumRegistrations += RegisterServiceClass(RegisterClass, InstanceClass, Config.GetPriority(), TemplateInstance);
	}

	UE_LOG(LogGameService, Log, TEXT("Registered %d service classes from %s (Priority: %d)"),
		NumRegistrations, *Config.GetClass()->GetName(), Config.GetPriority());
}

bool UGameServiceManager::RegisterServiceClass(const FGameServiceClass& ServiceClass, const FGameServiceInstanceClass& InstanceClass, int32 Priority, const UGameServiceBase* TemplateInstance)
{
	if (WasServiceStarted(ServiceClass))
	{
		UE_LOG(LogGameService, Log,
			TEXT("RegisterServiceClass<%s, %s> was ignored because of already running service (%s)."),
			*GetNameSafe(ServiceClass), *GetNameSafe(InstanceClass), *GetNameSafe(FindStartedServiceInstance(ServiceClass)));
		return false;
	}

	FServiceClassRegistryEntry& RegisteredEntry = RegisterServiceClassInternal(ServiceClass, InstanceClass);
	if (RegisteredEntry.InstanceClass == InstanceClass)
		return false; // Already registered.

	if (RegisteredEntry.InstanceClass != nullptr)
	{
		// Class already registered with higher priority:
		if (RegisteredEntry.RegisterPriority > Priority)
			return false;

		// Priority is equal -> overwrite existing entry but log a warning:
		UE_CLOG((RegisteredEntry.RegisterPriority == Priority), LogGameService, Warning,
			TEXT("RegisterServiceClass<%s, %s> has the same priority (%d) as an already registered entry (%s). The new entry will overwrite the existing one."),
			*GetNameSafe(ServiceClass), *GetNameSafe(InstanceClass), Priority, *GetNameSafe(RegisteredEntry.InstanceClass));
	}

	UE_LOG(LogGameService, Verbose, TEXT("Already registered game service [%s | %s] will be overidden by higher priority registration -> [%s | %s]"),
		*GetNameSafe(RegisteredEntry.RegisterClass), *GetNameSafe(RegisteredEntry.InstanceClass),
		*GetNameSafe(ServiceClass), *GetNameSafe(InstanceClass));

	RegisteredEntry.RegisterClass = ServiceClass;
	RegisteredEntry.InstanceClass = InstanceClass;
	RegisteredEntry.InstanceTemplate = TemplateInstance;
	RegisteredEntry.RegisterPriority = Priority;
	return true;
}

void UGameServiceManager::StartRegisteredServices(UWorld& TargetWorld)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGameServiceManager.StartRegisteredServices"), STAT_GameServiceManager_StartRegisteredServices, STATGROUP_GameService);
	UE_LOG(LogGameService, Log, TEXT(">>> Starting registered game services for world (%s):"), *TargetWorld.GetName());

	for (auto RegisterItr = ServiceClassRegisters.CreateIterator(); RegisterItr; ++RegisterItr)
	{
		TArray<FServiceClassRegistryEntry> RegisteredServices;
		RegisterItr.Value().GenerateValueArray(OUT RegisteredServices);
		for (const FServiceClassRegistryEntry& ServiceEntry : RegisteredServices)
		{
			StartService(TargetWorld, ServiceEntry.RegisterClass, ServiceEntry.InstanceClass, ServiceEntry.InstanceTemplate.Get());
		}
	}

	UE_LOG(LogGameService, Log, TEXT("<<< Finished starting registered game services for world (%s)"), *TargetWorld.GetName());
}

UGameServiceBase& UGameServiceManager::StartService(UWorld& TargetWorld, const FGameServiceClass& ServiceClass, UGameServiceBase& ServiceInstance)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGameServiceManager.StartService"), STAT_GameServiceManager_StartService, STATGROUP_GameService);

	if (WasServiceStarted(ServiceClass))
	{
		UGameServiceBase* RunningInstance = StartedServices[ServiceClass].Get();
		ensure(RunningInstance == &ServiceInstance);
		UE_LOG(LogGameService, Verbose, TEXT("Game service already running: [%s] %s"), *ServiceClass->GetName(), *ServiceInstance.GetName());
		return *RunningInstance;
	}

	FServiceClassRegistryEntry& RegisteredEntry = RegisterServiceClassInternal(ServiceClass, ServiceInstance);
	if (RegisteredEntry.InstanceClass != nullptr)
	{
		ensureMsgf(ServiceInstance.GetClass() == RegisteredEntry.InstanceClass,
			TEXT("Starting a service (%s | %s) that was previously registered for a different instance class (%s)"),
			*ServiceClass->GetName(), * ServiceInstance.GetClass()->GetName(), *RegisteredEntry.InstanceClass->GetName());
	}

	RegisteredEntry.RegisterClass = ServiceClass;
	RegisteredEntry.InstanceClass = ServiceInstance.GetClass();
	StartedServices.Add(ServiceClass, TStrongObjectPtr(&ServiceInstance));

	// First start all dependency services:
	StartServiceDependencies(TargetWorld, ServiceInstance);

	// Now start the actual service:
	UE_LOG(LogGameService, Log, TEXT("#%3d | Start game service: [%s] %s"), StartOrderedServices.Num(), *ServiceClass->GetName(), *ServiceInstance.GetName());
	ServiceInstance.StartService();

	// Only now note down that the service was started, because dependency services
	// will recursively run before and thus get a lower index in the array:
	StartOrderedServices.Add(ServiceClass);

	return ServiceInstance;
}

UGameServiceBase& UGameServiceManager::StartService(UWorld& TargetWorld, const FGameServiceClass& ServiceClass, const FGameServiceInstanceClass& InstanceClass, const UGameServiceBase* TemplateInstance)
{
	// Was a service with the same ServiceClass already started, then return its instance:
	if (WasServiceStarted(ServiceClass))
	{
		UGameServiceBase* RunningInstance = StartedServices[ServiceClass].Get();
		ensure(InstanceClass == RunningInstance->GetClass());
		return *RunningInstance;
	}

	// Was a service with the same InstanceClass already started, but under a different ServiceClass,
	// then re-use the existing instance but allow it to be also found by the new ServiceClass:
	for (auto Itr = StartedServices.CreateIterator(); Itr; ++Itr)
	{
		const FGameServiceClass& StartedServiceClass = Itr.Key();
		const TStrongObjectPtr<UGameServiceBase> StartedServiceInstance = Itr.Value();
		const bool bIsSameInstanceClass = (StartedServiceInstance->GetClass() == InstanceClass);
		const bool bIsDifferentRegisterClass = (StartedServiceClass != ServiceClass);

		if (bIsSameInstanceClass && bIsDifferentRegisterClass)
		{
			UE_LOG(LogGameService, Log, TEXT(">%3d | Alias game service: [%s] %s"),
				StartOrderedServices.IndexOfByKey(StartedServiceClass), *ServiceClass->GetName(), *StartedServiceInstance->GetName());

			StartedServices.Add(ServiceClass, StartedServiceInstance);
			FServiceClassRegistryEntry& RegisteredEntry = RegisterServiceClassInternal(ServiceClass, InstanceClass);
			RegisteredEntry.RegisterClass = ServiceClass;
			RegisteredEntry.InstanceClass = InstanceClass;

			return *StartedServiceInstance;
		}
	}

	// Create a new instance and start it:
	UObject* ServiceOwner;
	switch (UGameServiceBase::GetLifetimeOf(InstanceClass))
	{
		default:
		case EGameServiceLifetime::ShutdownWithWorld:
			ServiceOwner =  &TargetWorld;
			break;

		case EGameServiceLifetime::ShutdownWithGameInstance:
			ServiceOwner = TargetWorld.GetGameInstance();
			break;
	}
	return StartService(TargetWorld, ServiceClass, *CreateServiceInstance(*ServiceOwner, InstanceClass, TemplateInstance));
}

UGameServiceBase& UGameServiceManager::StartService(UWorld& TargetWorld, const FGameServiceInstanceClass& InstanceClass)
{
	if (WasServiceStarted(InstanceClass))
		return *StartedServices[InstanceClass].Get();

	return StartService(TargetWorld, InstanceClass, InstanceClass);
}

TOptional<UGameServiceBase*> UGameServiceManager::TryStartService(UWorld& TargetWorld, const FGameServiceClass& ServiceClass)
{
	if (WasServiceStarted(ServiceClass))
		return StartedServices[ServiceClass].Get();

	TOptional<FGameServiceInstanceClass> ServiceInstanceClass = DetermineServiceInstanceClass(ServiceClass);
	if (!ServiceInstanceClass.IsSet())
		return {};

	return &StartService(TargetWorld, ServiceClass, *ServiceInstanceClass);
}

bool UGameServiceManager::IsServiceRegistered(const FGameServiceClass& ServiceClass) const
{
	for (auto RegisterItr = ServiceClassRegisters.CreateConstIterator(); RegisterItr; ++RegisterItr)
	{
		if (RegisterItr.Value().Contains(ServiceClass))
			return true;
	}
	return false;
}

bool UGameServiceManager::WasServiceStarted(const FGameServiceClass& ServiceClass) const
{
	return StartedServices.Contains(ServiceClass);
}

bool UGameServiceManager::WasServiceStarted(const UGameServiceBase* ServiceInstance) const
{
	return (StartedServices.FindKey(TStrongObjectPtr(const_cast<UGameServiceBase*>(ServiceInstance))) != nullptr);
}

bool UGameServiceManager::IsServiceRunning(const FGameServiceClass& ServiceClass) const
{
	if (!WasServiceStarted(ServiceClass))
		return false;

	const UAsyncGameServiceBase* AsyncService = Cast<UAsyncGameServiceBase>(FindStartedServiceInstance(ServiceClass));
	return (!IsValid(AsyncService) || AsyncService->IsServiceRunning());
}

bool UGameServiceManager::IsServiceRunning(const UGameServiceBase* ServiceInstance) const
{
	if (!WasServiceStarted(ServiceInstance))
		return false;

	const UAsyncGameServiceBase* AsyncService = Cast<UAsyncGameServiceBase>(ServiceInstance);
	return (!IsValid(AsyncService) || AsyncService->IsServiceRunning());
}

UGameServiceBase* UGameServiceManager::FindStartedServiceInstance(const FGameServiceClass& ServiceClass) const
{
	const TStrongObjectPtr<UGameServiceBase>* RunningInstance = StartedServices.Find(ServiceClass);
	return (RunningInstance ? RunningInstance->Get() : nullptr);
}

TArray<FGameServiceClass> UGameServiceManager::GetAllRegisteredServiceClasses() const
{
	TArray<FGameServiceClass> Result;
	for (auto RegisterItr = ServiceClassRegisters.CreateConstIterator(); RegisterItr; ++RegisterItr)
	{
		TArray<FGameServiceClass> IntermediateResult;
		RegisterItr.Value().GenerateKeyArray(OUT IntermediateResult);
		Result += IntermediateResult;
	}
	return Result;
}

TArray<FGameServiceInstanceClass> UGameServiceManager::GetAllRegisteredServiceInstanceClasses() const
{
	TArray<FGameServiceInstanceClass> Result;
	for (auto RegisterItr = ServiceClassRegisters.CreateConstIterator(); RegisterItr; ++RegisterItr)
	{
		TArray<FGameServiceInstanceClass> IntermediateResult;
		TArray<FServiceClassRegistryEntry> Entries;
		RegisterItr.Value().GenerateValueArray(OUT Entries);
		Algo::Transform(Entries, OUT IntermediateResult, &FServiceClassRegistryEntry::InstanceClass);
		Result += IntermediateResult;
	}
	return Result;
}

TOptional<FGameServiceInstanceClass> UGameServiceManager::FindRegisteredServiceInstanceClass(const FGameServiceClass& ServiceClass) const
{
	for (auto RegisterItr = ServiceClassRegisters.CreateConstIterator(); RegisterItr; ++RegisterItr)
	{
		if (const FServiceClassRegistryEntry* Entry = RegisterItr.Value().Find(ServiceClass))
			return Entry->InstanceClass;
	}
	return TOptional<FGameServiceInstanceClass>();
}

TArray<FGameServiceClass> UGameServiceManager::GetAllStartedServiceClasses() const
{
	return StartOrderedServices;
}

TOptional<FGameServiceInstanceClass> UGameServiceManager::DetermineServiceInstanceClass(const FGameServiceClass& ServiceClass) const
{
	if (TOptional<FGameServiceInstanceClass> RegisteredInstanceClass = FindRegisteredServiceInstanceClass(ServiceClass); RegisteredInstanceClass.IsSet())
		return RegisteredInstanceClass;

	if (ServiceClass->IsChildOf<UGameServiceBase>())
		return FGameServiceInstanceClass(*ServiceClass);

	return {};
}

TArray<UGameServiceBase*> UGameServiceManager::GetAllStartedServiceInstances() const
{
	TArray<UGameServiceBase*> Result;
	Result.Reserve(StartOrderedServices.Num());
	for (const FGameServiceClass& ServiceClass : StartOrderedServices)
	{
		Result.Add(StartedServices[ServiceClass].Get());
	}
	return Result;
}

void UGameServiceManager::ShutdownAllServices()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGameServiceManager.ShutdownAllServices"), STAT_GameServiceManager_ShutdownAllServices, STATGROUP_GameService);

	// Shutdown the services in the reverse order of how they were started:
	while (StartOrderedServices.Num() > 0)
	{
		TStrongObjectPtr<UGameServiceBase> ServiceToShutdown;
		StartedServices.RemoveAndCopyValue(StartOrderedServices.Pop(), OUT ServiceToShutdown);

		UE_LOG(LogGameService, Log, TEXT("#%3d | Shutdown game service: %s"), StartOrderedServices.Num(), *GetNameSafe(ServiceToShutdown.Get()));
		ServiceToShutdown->ShutdownService();
	}

	// Service instances registered under multiple service classes have been shut down,
	// but the aliased entries still remain in the list, so let's clear it:
	StartedServices.Empty();
}

void UGameServiceManager::ShutdownAllServicesWithLifetime(const EGameServiceLifetime& Lifetime)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGameServiceManager.ShutdownAllServicesWithLifetime"), STAT_GameServiceManager_ShutdownAllServicesWithLifetime, STATGROUP_GameService);

	// Shutdown the services in the reverse order of how they were started:
	for (int32 i = 0; i < StartOrderedServices.Num(); ++i)
	{
		const FGameServiceClass& ServiceClass = StartOrderedServices[i];
		if (StartedServices[ServiceClass]->GetLifetime() != Lifetime)
			continue;

		StartOrderedServices.RemoveAt(i--);
		TStrongObjectPtr<UGameServiceBase> ServiceToShutdown = StartedServices[ServiceClass];
		while (const FGameServiceClass* RegisteredServiceClass = StartedServices.FindKey(ServiceToShutdown))
		{
			// Remove all classes & aliases associated with the instance: 
			StartedServices.Remove(*RegisteredServiceClass);
		}

		UE_LOG(LogGameService, Log, TEXT("#%3d | Shutdown game service: %s"), StartOrderedServices.Num(), *GetNameSafe(ServiceToShutdown.Get()));
		ServiceToShutdown->ShutdownService();
	}
}

void UGameServiceManager::ClearServiceRegister(const EGameServiceLifetime& Lifetime)
{
	ServiceClassRegisters.FindOrAdd(Lifetime).Empty();
	UE_LOG(LogGameService, Log, TEXT("Service register for %s was cleared."), *LexToString(Lifetime));
}

UGameServiceBase* UGameServiceManager::CreateServiceInstance(UObject& Owner, const FGameServiceClass& ServiceInstanceClass, const UGameServiceBase* TemplateInstance)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGameServiceManager.CreateServiceInstance"), STAT_GameServiceManager_CreateServiceInstance, STATGROUP_GameService);
	const FName InstanceName = MakeUniqueObjectName(&Owner, ServiceInstanceClass);
	return NewObject<UGameServiceBase>(&Owner, ServiceInstanceClass, InstanceName, RF_NoFlags, const_cast<UGameServiceBase*>(TemplateInstance));
}

void UGameServiceManager::StartServiceDependencies(UWorld& TargetWorld, const UGameServiceBase& ServiceInstance)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGameServiceManager.StartServiceDependencies"), STAT_GameServiceManager_StartServiceDependencies, STATGROUP_GameService);

	for (const FGameServiceClass& ServiceDependency : ServiceInstance.GetServiceClassDependencies())
	{
		if (WasServiceStarted(ServiceDependency))
			continue;

		UE_LOG(LogGameService, Verbose, TEXT("- Start dependency for game service [%s] -> [%s]"), *ServiceInstance.GetName(), *ServiceDependency->GetName());
		const bool bCouldStartDependency = TryStartService(TargetWorld, ServiceDependency).IsSet();
		checkf(bCouldStartDependency, TEXT("No appropriate service instance class found for %s"), *GetNameSafe(ServiceDependency));
		// If the above check triggers, then we probably have a service that depends on another service that was never configured
		// in the UGameServiceConfig for the currently running map. => The service manager doesn't know which UGameService to instance.
	}
}

UGameServiceManager::FServiceClassRegistryEntry& UGameServiceManager::RegisterServiceClassInternal(const FGameServiceClass& ServiceClass, const FGameServiceInstanceClass& InstanceClass)
{
	const EGameServiceLifetime Lifetime = UGameServiceBase::GetLifetimeOf(InstanceClass);
	return ServiceClassRegisters.FindOrAdd(Lifetime).FindOrAdd(ServiceClass);
}

UGameServiceManager::FServiceClassRegistryEntry& UGameServiceManager::RegisterServiceClassInternal(const FGameServiceClass& ServiceClass, const UGameServiceBase& ServiceInstance)
{
	const EGameServiceLifetime Lifetime = ServiceInstance.GetLifetime();
	return ServiceClassRegisters.FindOrAdd(Lifetime).FindOrAdd(ServiceClass);
}
