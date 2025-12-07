///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#if WITH_GAMEPLAY_DEBUGGER

#include "GameplayDebuggerCategory_GameServices.h"

#include "Algo/ForEach.h"
#include "GameService/GameServiceManager.h"

//#CVar gdt.Category.GameServices.ShowDependencies
static TAutoConsoleVariable<bool> CVar_GameServicesDebugger_ShowDependencies(
	TEXT("gdt.Category.GameServices.ShowDependencies"), true,
	TEXT("Enable to show information about game service dependencies in the [GameServices] gameplay debugger."));

FGameplayDebuggerCategory_GameServices::FGameplayDebuggerCategory_GameServices()
{
	bShowCategoryName = false;
	bShowOnlyWithDebugActor = false;
	CollectDataInterval = 0.5f;

	BindKeyPress(EKeys::D.GetFName(), FGameplayDebuggerInputModifier::Ctrl, this, &FGameplayDebuggerCategory_GameServices::ToggleShowDependencies, EGameplayDebuggerInputMode::Local);
}

void FGameplayDebuggerCategory_GameServices::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	const UGameServiceManager* GameServiceManager = UGameServiceManager::FindInstance(OwnerPC);
	if (!IsValid(GameServiceManager))
	{
		AddTextLine("{red}<No GameServiceManager>");
		return;
	}

	////////////////////////////////////////////////////////////////////////////////
	// Collect information:
	const TArray<FGameServiceClass> StartedServiceClasses = GameServiceManager->GetAllStartedServiceClasses();
	TArray<FGameServiceClass> RegisteredServiceClasses = GameServiceManager->GetAllRegisteredServiceClasses();
	RegisteredServiceClasses.Sort([StartedServiceClasses](const FGameServiceClass& Lhs, const FGameServiceClass& Rhs)
	{
		// Services that were not started will sorted by name to the front of the list,
		// while started services come next sorted by their start index:
		const int32 IndexOfLhs = StartedServiceClasses.IndexOfByKey(Lhs);
		const int32 IndexOfRhs = StartedServiceClasses.IndexOfByKey(Rhs);
		return (IndexOfLhs == IndexOfRhs) ? (Lhs < Rhs) : (IndexOfLhs < IndexOfRhs);
	});

	TArray<FString> OfflineServicesInfo, StartedServicesInfo, AliasedServicesInfo;
	for (const FGameServiceClass& ServiceClass : RegisteredServiceClasses)
	{
		const FGameServiceInstanceClass& InstanceClass = *GameServiceManager->FindRegisteredServiceInstanceClass(ServiceClass);
		const UGameServiceBase* ServiceInstance = GameServiceManager->FindStartedServiceInstance(ServiceClass);
		const FString StartOrderInfo = FString::Printf(TEXT("#%3d"), StartedServiceClasses.IndexOfByKey(ServiceClass));

		//////////////////////////////////////////////////////////
		// Not yet started:
		if (!GameServiceManager->WasServiceStarted(ServiceClass))
		{
			OfflineServicesInfo.Add(FString::Printf(TEXT("{grey}<Offline> {white}[{yellow}%s{white} | %s]"), *GetNameSafe(ServiceClass), *GetNameSafe(InstanceClass)));
			OfflineServicesInfo += CollectServiceDependenciesInfo(*GameServiceManager, ServiceClass);
			continue;
		}

		//////////////////////////////////////////////////////////
		// Alias:
		if (!StartedServiceClasses.Contains(ServiceClass))
		{
			AliasedServicesInfo.Add(FString::Printf(TEXT("{white}<Alias> [{yellow}%s{white} | %s]"), *GetNameSafe(ServiceClass), *GetNameSafe(InstanceClass)));
			continue;
		}

		//////////////////////////////////////////////////////////
		// Started / Running:
		StartedServicesInfo.Add(FString::Printf(TEXT("%s {white}%s [{yellow}%s{white} | %s] {yellow}%s"),
			GameServiceManager->IsServiceRunning(ServiceClass) ? TEXT("{green}<Running>") : TEXT("{orange}<Starting>"),
			*StartOrderInfo, *GetNameSafe(ServiceClass), *GetNameSafe(InstanceClass),
			*(IsValid(ServiceInstance) ? ServiceInstance->GetServiceStatusInfo().Get(FString()) : FString())));
		StartedServicesInfo += CollectServiceDependenciesInfo(*GameServiceManager, ServiceClass);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Print collected information:
	AddTextLine("");
	AddTextLine(FString::Printf(TEXT("{white}({cyan}CRTL + D{white}) Show Dependencies [%s{white}]"), ShouldShowDependencies() ? TEXT("{green}ON") : TEXT("{grey}OFF")));
	if (RegisteredServiceClasses.IsEmpty())
	{
		AddTextLine("{white}------------------------------------");
		AddTextLine(  "{red}     No Game Services registered.");
		AddTextLine("{white}------------------------------------");
	}
	else
	{
		AddTextLine("{white}------------------------------------");
		AddTextLine("{white}       Registered Game Services:");
		AddTextLine("{white}------------------------------------");
		Algo::ForEach(OfflineServicesInfo, [this](const FString& Line) { AddTextLine(Line); });
		Algo::ForEach(StartedServicesInfo, [this](const FString& Line) { AddTextLine(Line); });
	}
	if (AliasedServicesInfo.Num() > 0)
	{
		AddTextLine("{white}------------------------------------");
		AddTextLine("{white}       Aliased Game Services:");
		AddTextLine("{white}------------------------------------");
		Algo::ForEach(AliasedServicesInfo, [this](const FString& Line) { AddTextLine(Line); });
	}
}

TArray<FString> FGameplayDebuggerCategory_GameServices::CollectServiceDependenciesInfo(const UGameServiceManager& GameServiceManager, const FGameServiceClass& ServiceClass) const
{
	if (!ShouldShowDependencies())
		return {};

	TOptional<FGameServiceInstanceClass> InstanceClass = GameServiceManager.FindRegisteredServiceInstanceClass(ServiceClass);
	if (!InstanceClass.IsSet())
		return {"{white}\t+ Dependencies: {red}???"};

	TArray<FString> DependenciesInfo;
	const UGameServiceBase* ServiceClassCdo = InstanceClass.GetValue()->GetDefaultObject<UGameServiceBase>();

	//////////////////////////////////////////////////////////
	// Service dependencies:
	if (const TArray<FGameServiceClass>& Dependencies = ServiceClassCdo->GetServiceClassDependencies(); Dependencies.Num() > 0)
	{
		DependenciesInfo.Add_GetRef("{white}\t+ Depends on Services: ")
			+= FString::JoinBy(Dependencies, TEXT("{white}, "), [&GameServiceManager](const FGameServiceClass& Dependency) {
				return FString::Printf(TEXT("{%s}%s"), *BoolToCyanOrOrange(GameServiceManager.IsServiceRunning(Dependency)), *GetNameSafe(Dependency));
			});
	}

	//////////////////////////////////////////////////////////
	// Subsystem dependencies:
	if (const TArray<TSubclassOf<USubsystem>>& Dependencies = ServiceClassCdo->GetSubsystemClassDependencies(); Dependencies.Num() > 0)
	{
		DependenciesInfo.Add_GetRef("{white}\t+ Depends on Subsystems: ")
			+= FString::JoinBy(Dependencies, TEXT("{white}, "), [](const TSubclassOf<USubsystem>& Dependency) {
				TArray<UObject*> DependencyInstances; GetObjectsOfClass(Dependency, OUT DependencyInstances);
				return FString::Printf(TEXT("{%s}%s"), *BoolToCyanOrOrange(DependencyInstances.Num() > 0), *GetNameSafe(Dependency));
			});
	}

	//////////////////////////////////////////////////////////
	// Optional service dependencies:
	if (const TArray<TSubclassOf<USubsystem>>& Dependencies = ServiceClassCdo->GetOptionalSubsystemClassDependencies(); Dependencies.Num() > 0)
	{
		DependenciesInfo.Add_GetRef("{white}\t+ Can depend on Subsytems: ")
			+= FString::JoinBy(Dependencies, TEXT("{white}, "), [](const TSubclassOf<USubsystem>& Dependency) {
				TArray<UObject*> DependencyInstances; GetObjectsOfClass(Dependency, OUT DependencyInstances);
				return FString::Printf(TEXT("{%s}%s"), *BoolToCyanOrOrange(DependencyInstances.Num() > 0), *GetNameSafe(Dependency));
			});
	}

	return DependenciesInfo;
}

void FGameplayDebuggerCategory_GameServices::ToggleShowDependencies()
{
	CVar_GameServicesDebugger_ShowDependencies->Set(!ShouldShowDependencies());
}

bool FGameplayDebuggerCategory_GameServices::ShouldShowDependencies()
{
	return CVar_GameServicesDebugger_ShowDependencies->GetBool();
}

FString FGameplayDebuggerCategory_GameServices::BoolToCyanOrOrange(bool bUseCyanOverOrange)
{
	return (bUseCyanOverOrange ? FColor::Cyan.ToString() : FColor::Orange.ToString());
}

#endif
