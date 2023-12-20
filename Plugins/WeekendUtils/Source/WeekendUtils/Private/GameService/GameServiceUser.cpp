///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "GameService/GameServiceUser.h"

#include "GameService/GameServiceBase.h"
#include "GameService/GameServiceManager.h"
#include "Initialization/WorldGameServiceRunner.h"
#include "WeekendUtils.h"

#include "Subsystems/EngineSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Subsystems/WorldSubsystem.h"

const TArray<FGameServiceUser::FGameServiceClass>& FGameServiceUser::GetServiceClassDependencies() const
{
	return ServiceDependencies.ToArray();
}

const TArray<TSubclassOf<USubsystem>>& FGameServiceUser::GetSubsystemClassDependencies() const
{
	return SubsystemDependencies.ToArray();
}

const TArray<TSubclassOf<USubsystem>>& FGameServiceUser::GetOptionalSubsystemClassDependencies() const
{
	return OptionalSubsystemDependencies.ToArray();
}

bool FGameServiceUser::AreAllDependenciesReady(const UObject* ServiceUser) const
{
	return (AreServiceDependenciesReady() && AreSubsystemDependenciesReady(ServiceUser));
}

bool FGameServiceUser::AreServiceDependenciesReady() const
{
	const UGameServiceManager& ServiceManager = UGameServiceManager::Get();
	for (const FGameServiceClass& ServiceClass : ServiceDependencies.ToArray())
	{
		if (!ServiceManager.IsServiceRunning(ServiceClass))
			return false;
	}
	return true;
}

bool FGameServiceUser::AreSubsystemDependenciesReady(const UObject* ServiceUser) const
{
	for (const TSubclassOf<USubsystem>& SubsystemClass : SubsystemDependencies.ToArray())
	{
		TWeakObjectPtr<const USubsystem> SubsystemInstance = FindSubsystemDependency(ServiceUser, *SubsystemClass);
		if (!SubsystemInstance.IsValid())
			return false;
	}
	for (const TSubclassOf<USubsystem>& SubsystemClass : OptionalSubsystemDependencies.ToArray())
	{
		TWeakObjectPtr<const USubsystem> SubsystemInstance = FindSubsystemDependency(ServiceUser, *SubsystemClass);
		if (!SubsystemInstance.IsValid())
		{
			// Optional subsystems might not be available in the current environment, skip those,
			// because we consider them "ready":
			if (SubsystemClass->GetDefaultObject<USubsystem>()->ShouldCreateSubsystem(nullptr))
				return false;
		}
	}
	return true;
}

void FGameServiceUser::WaitForDependencies(const UObject* ServiceUser, FOnWaitingFinished Callback)
{
	if (!AreServiceDependenciesReady())
	{
		// Start all service dependencies, which just happens:
		UGameServiceManager& ServiceManager = UGameServiceManager::Get();
		UWorld& ServiceWorld = *ServiceUser->GetWorld();
		for (const FGameServiceClass& ServiceClass : ServiceDependencies.ToArray())
		{
			const bool bWasDependencyStarted = ServiceManager.TryStartService(ServiceWorld, ServiceClass).IsSet();
			ensureMsgf(bWasDependencyStarted, TEXT("%s is waiting for dependency %s, which could not be started. Is %s properly configured?"),
				*GetNameSafe(ServiceUser), *ServiceClass->GetName(), *ServiceClass->GetName());
		}
	}

	if (AreAllDependenciesReady(ServiceUser))
	{
		Callback.ExecuteIfBound();
		return;
	}

	// Wait for subsystem dependencies, which are started whenever:
	PendingDependencyWaitCallbacks.Add(Callback);
	PollPendingDependencyWaitCallbacks(ServiceUser);
}

void FGameServiceUser::WaitForDependencies(const UObject* ServiceUser, TFunction<void()> Callback)
{
	WaitForDependencies(ServiceUser, FOnWaitingFinished::CreateWeakLambda(ServiceUser, Callback));
}

void FGameServiceUser::InitializeWorldSubsystemDependencies_Internal(FSubsystemCollectionBase& SubsystemCollection)
{
	SubsystemCollection.InitializeDependency<UWorldGameServiceRunner>();
	for (const TSubclassOf<USubsystem>& SubsystemDependency : SubsystemDependencies.ToArray())
	{
		if (SubsystemDependency->IsChildOf<UWorldSubsystem>())
		{
			SubsystemCollection.InitializeDependency(SubsystemDependency);
		}
	}
	for (const TSubclassOf<USubsystem>& SubsystemDependency : OptionalSubsystemDependencies.ToArray())
	{
		if (SubsystemDependency->IsChildOf<UWorldSubsystem>())
		{
			SubsystemCollection.InitializeDependency(SubsystemDependency);
		}
	}
}

UObject* FGameServiceUser::UseGameService_Internal(const UObject* ServiceUser, const TSubclassOf<UObject>& ServiceClass) const
{
	return &UseGameService(ServiceUser, ServiceClass);
}

UObject* FGameServiceUser::FindOptionalGameService_Internal(const FGameServiceClass& ServiceClass) const
{
	const UGameServiceManager* ServiceManager = UGameServiceManager::GetPtr();
	return (IsValid(ServiceManager) ? ServiceManager->FindStartedServiceInstance(ServiceClass) : nullptr);
}

UGameServiceBase& FGameServiceUser::UseGameService(const UObject* ServiceUser, const FGameServiceClass& ServiceClass) const
{
	// (!) ServiceDependencies must be registered for GameServiceUsers.
	ensureMsgf(ServiceDependencies.Contains(ServiceClass),
		TEXT("UseGameService<%s>() was called, but service is not registered as dependency."),
		*ServiceClass->GetName());

	// (!) World subsystem service users should call InitializeWorldSubsystemDependencies() before accessing
	// game services to ensure the used services were configured correctly before.
	if (ServiceUser->IsA<UWorldSubsystem>())
	{
		const UWorldGameServiceRunner* ServiceRunner = ServiceUser->GetWorld()->GetSubsystem<UWorldGameServiceRunner>();
		const bool bServiceRunnerIsInitialized = (IsValid(ServiceRunner) && ServiceRunner->IsInitialized());
		ensureMsgf(bServiceRunnerIsInitialized,
			TEXT("%s is a UWorldSubsystem trying to use game services before the UWorldGameServiceRunner was initialized. ")
			TEXT("Please call InitializeWorldSubsystemDependencies() before using services."),
			*ServiceUser->GetClass()->GetName());
	}

	UGameServiceManager& ServiceManager = UGameServiceManager::Get();

	TOptional<FGameServiceInstanceClass> ServiceInstanceClass = ServiceManager.DetermineServiceInstanceClass(ServiceClass);
	checkf(ServiceInstanceClass.IsSet(), TEXT("No appropriate service instance class found for %s"), *GetNameSafe(ServiceClass));
	// If the above check triggers, then our service dependency was probably configured via interface, but nobody told the service manager
	// which UGameService class should be instanced for the interface. Check the UGameServiceConfig for the currently running map.

	return ServiceManager.StartService(*ServiceUser->GetWorld(), ServiceClass, *ServiceInstanceClass);
}

TWeakObjectPtr<UGameServiceBase> FGameServiceUser::FindOptionalGameService(const FGameServiceClass& ServiceClass) const
{
	const UGameServiceManager* ServiceManager = UGameServiceManager::GetPtr();
	return (IsValid(ServiceManager) ? MakeWeakObjectPtr(ServiceManager->FindStartedServiceInstance(ServiceClass)) : nullptr);
}

TWeakObjectPtr<USubsystem> FGameServiceUser::FindSubsystemDependency(const UObject* ServiceUser, const TSubclassOf<USubsystem>& SubsystemClass) const
{
	// (!) SubsystemDependencies should be registered for GameServiceUsers, but not as strictly as ServiceDependencies, so only log a warning:
	const bool bWarnAboutMissingConfig = (!SubsystemDependencies.Contains(SubsystemClass) && !OptionalSubsystemDependencies.Contains(SubsystemClass));
	UE_CLOG(bWarnAboutMissingConfig, LogGameService, Warning, TEXT("ServiceUser %s accesses %s, which was never configured as SubsystemDependency"),
		*GetNameSafe(ServiceUser), *GetNameSafe(SubsystemClass));

	auto Sanitize = [SubsystemClass](USubsystem* Subsystem) -> TWeakObjectPtr<USubsystem>
	{
		// (i) This looks stupid, but when you use GetSubsystemBase() and no instance of the desired subsystem class
		// was ever created (i.e. because of ShouldCreateSubsystem() returns false in some environments), then the API
		// just gives you the first subsystem it can find, regardless of inheritance. WTF.
		return (IsValid(Subsystem) && Subsystem->GetClass() == SubsystemClass) ? MakeWeakObjectPtr(Subsystem) : nullptr;
	};

	// [ENGINE]
	if (SubsystemClass->IsChildOf<UEngineSubsystem>())
	{
		return Sanitize(GEngine ? GEngine->GetEngineSubsystemBase(*SubsystemClass) : nullptr);
	}

	const UWorld* ServiceWorld = ServiceUser->GetWorld();
	if (!IsValid(ServiceWorld))
		return nullptr;

	// [WORLD]
	if (SubsystemClass->IsChildOf<UWorldSubsystem>())
	{
		return Sanitize(ServiceWorld->GetSubsystemBase(*SubsystemClass));
	}

	// [GAME INSTANCE]
	if (SubsystemClass->IsChildOf<UGameInstanceSubsystem>())
	{
		const UGameInstance* GameInstance = ServiceWorld->GetGameInstance();
		return Sanitize(IsValid(GameInstance) ? GameInstance->GetSubsystemBase(*SubsystemClass) : nullptr);
	}

	// [LOCAL PLAYER]
	if (SubsystemClass->IsChildOf<ULocalPlayerSubsystem>())
	{
		const ULocalPlayer* FirstLocalPlayer = ServiceWorld->GetFirstPlayerController()->GetLocalPlayer();
		return Sanitize(IsValid(FirstLocalPlayer) ? FirstLocalPlayer->GetSubsystemBase(*SubsystemClass) : nullptr);
	}

	// (!) EditorSubsystems are not supported for game services.
	checkf(false, TEXT("GameServiceUser can only find subsystem dependencies of supported classes, not %s"), *GetNameSafe(SubsystemClass));
	return nullptr;
}

bool FGameServiceUser::IsGameServiceRegistered(const FGameServiceClass& ServiceClass) const
{
	return UGameServiceManager::Get().IsServiceRegistered(ServiceClass);
}

void FGameServiceUser::PollPendingDependencyWaitCallbacks(const UObject* ServiceUser)
{
	if (!IsValid(ServiceUser) || !IsValid(ServiceUser->GetWorld()))
		return; // User died while waiting.

	// Notify waiting objects when dependencies are ready:
	if (AreAllDependenciesReady(ServiceUser))
	{
		while (PendingDependencyWaitCallbacks.Num() > 0)
		{
			PendingDependencyWaitCallbacks.Pop().ExecuteIfBound();
		}
		return;
	}

	// Keep polling:
	FTimerManager& Timer = ServiceUser->GetWorld()->GetTimerManager();
	Timer.SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(ServiceUser, [this, ServiceUser]()
	{
		PollPendingDependencyWaitCallbacks(ServiceUser);
	}));
}

void FGameServiceUser::StopWaitingForDependencies(const UObject* ServiceUser)
{
	if (!IsValid(ServiceUser) || !IsValid(ServiceUser->GetWorld()))
		return;

	ServiceUser->GetWorld()->GetTimerManager().ClearAllTimersForObject(ServiceUser);
	PendingDependencyWaitCallbacks.Empty();
}
