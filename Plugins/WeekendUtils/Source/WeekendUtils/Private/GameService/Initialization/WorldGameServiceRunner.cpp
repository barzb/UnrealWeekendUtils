///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz in cooperation with Nine Worlds Studios GmbH.
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See accompanying file LICENSE.
///
///////////////////////////////////////////////////////////////////////////////////////

#include "WorldGameServiceRunner.h"

#include "GameService/GameModeServiceConfigBase.h"
#include "GameService/GameServiceBase.h"
#include "GameService/GameServiceManager.h"
#include "WeekendUtils.h"

bool UWorldGameServiceRunner::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	switch (WorldType)
	{
		// Only create in game worlds:
		case EWorldType::Game:
		case EWorldType::PIE:
			return true;

		case EWorldType::None:
		case EWorldType::Editor:
		case EWorldType::EditorPreview:
		case EWorldType::GamePreview:
		case EWorldType::GameRPC:
		case EWorldType::Inactive:
		default: return false;
	}
}

void UWorldGameServiceRunner::Initialize(FSubsystemCollectionBase& Collection)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UWorldGameServiceRunner.Initialize"), STAT_WorldGameServiceRunner_Initialize, STATGROUP_GameService);

	Super::Initialize(Collection);

	RegisterAutoServiceConfig();

	// Initialize all other WorldSubsystems that any configured service depends on first:
	for (const TSubclassOf<UWorldSubsystem>& WorldSubsystemDependency : GatherWorldSubsystemDependencies())
	{
		Collection.InitializeDependency(WorldSubsystemDependency);
	}

	StartRegisteredServices();

	// 1. Gather GameServices that are registered as autonomous for this world
	// 2. Wait for any Subsystem dependencies on the autonomous services and their service dependencies (and the dependencies of the subsystem dependencies)
	// 3. Start all autonomous services and their service dependencies
}

void UWorldGameServiceRunner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TickRunningServices(DeltaTime);
}

void UWorldGameServiceRunner::TickRunningServices(float DeltaTime)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UWorldGameServiceRunner.TickRunningServices"), STAT_WorldGameServiceRunner_TickRunningServices, STATGROUP_GameService);

	for (UGameServiceBase* RunningService : UGameServiceManager::Get().GetAllStartedServiceInstances())
	{
		if (!IsValid(RunningService) || !RunningService->IsTickable())
			continue;

		//#todo-service-later #todo-benni DECLARE_SCOPE_CYCLE_COUNTER for all services that tick
		RunningService->TickService(DeltaTime);
	}
}

TStatId UWorldGameServiceRunner::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(STAT_WorldGameServiceStarter_Tick, STATGROUP_Tickables);
}

void UWorldGameServiceRunner::Deinitialize()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UWorldGameServiceRunner.Deinitialize"), STAT_WorldGameServiceRunner_Deinitialize, STATGROUP_GameService);

	// Clean up the ServiceManager, so the service runner in the next world can start anew:
	if (UGameServiceManager* ServiceManager = UGameServiceManager::GetPtr(); IsValid(ServiceManager))
	{
		ServiceManager->ShutdownAllServices();
		ServiceManager->ClearServiceRegister();
	}

	Super::Deinitialize();
}

void UWorldGameServiceRunner::RegisterAutoServiceConfig()
{
	if (const UGameModeServiceConfigBase* AutoConfig = UGameModeServiceConfigBase::FindConfigForWorld(*GetWorld()))
	{
		UE_LOG(LogGameService, Log, TEXT("Matching GameServiceConfig (%s) found for configured game mode in current world (%s)."),
			*GetNameSafe(AutoConfig->GetClass()), *GetNameSafe(GetWorld()));

		UGameServiceManager::Get().RegisterServices(*AutoConfig);
	}
}

void UWorldGameServiceRunner::StartRegisteredServices()
{
	UE_LOG(LogGameService, Verbose, TEXT("WorldGameServiceRunner will now start registered game services."));
	UGameServiceManager::Get().StartRegisteredServices(*GetWorld());
}

TArray<TSubclassOf<UWorldSubsystem>> UWorldGameServiceRunner::GatherWorldSubsystemDependencies()
{
	TArray<TSubclassOf<UWorldSubsystem>> Result;

	const TArray<FGameServiceInstanceClass> ServiceInstanceClasses = UGameServiceManager::Get().GetAllRegisteredServiceInstanceClasses();
	for (const FGameServiceInstanceClass& ServiceClass : ServiceInstanceClasses)
	{
		const auto GameServiceCDO = ServiceClass->GetDefaultObject<UGameServiceBase>();
		for (const TSubclassOf<USubsystem>& SubsystemDependency : GameServiceCDO->GetSubsystemClassDependencies())
		{
			if (!SubsystemDependency->IsChildOf<UWorldSubsystem>())
				continue;

			Result.AddUnique(*SubsystemDependency);
		}
	}

	return Result;
}
