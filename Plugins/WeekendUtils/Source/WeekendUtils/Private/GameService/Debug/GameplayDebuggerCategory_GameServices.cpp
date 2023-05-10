///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz in cooperation with Nine Worlds Studios GmbH.
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See accompanying file LICENSE.
///
///////////////////////////////////////////////////////////////////////////////////////

#if WITH_GAMEPLAY_DEBUGGER

#include "GameplayDebuggerCategory_GameServices.h"

#include "Algo/ForEach.h"
#include "GameService/GameServiceManager.h"
#include "GameplayDebuggerConfig.h"

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
	const UGameServiceManager* GameServiceManager = UGameServiceManager::GetPtr();
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
		int32 IndexOfLhs = StartedServiceClasses.IndexOfByKey(Lhs);
		int32 IndexOfRhs = StartedServiceClasses.IndexOfByKey(Rhs);
		return (IndexOfLhs == IndexOfRhs) ? (Lhs < Rhs) : (IndexOfLhs < IndexOfRhs);
	});

	TArray<FString> OfflineServicesInfo, StartedServicesInfo, AliasedServicesInfo;
	for (const FGameServiceClass& ServiceClass : RegisteredServiceClasses)
	{
		const FGameServiceInstanceClass& InstanceClass = *GameServiceManager->FindRegisteredServiceInstanceClass(ServiceClass);
		const UGameServiceBase* ServiceInstance = GameServiceManager->FindStartedServiceInstance(ServiceClass);

		//////////////////////////////////////////////////////////
		// Not yet started:
		if (!GameServiceManager->WasServiceStarted(ServiceClass))
		{
			OfflineServicesInfo.Add(FString::Printf(TEXT("{grey}<Offline> {white}[{yellow}%s{white} | %s]"), *GetNameSafe(ServiceClass), *GetNameSafe(InstanceClass)));
			OfflineServicesInfo += CollectServiceDependenciesInfo(ServiceClass);
			continue;
		}

		//////////////////////////////////////////////////////////
		// Alias:
		if (!StartedServiceClasses.Contains(ServiceClass))
		{
			AliasedServicesInfo.Add(FString::Printf(TEXT("{white}<alias> [{yellow}%s{white} | %s]"), *GetNameSafe(ServiceClass), *GetNameSafe(InstanceClass)));
			continue;
		}

		//////////////////////////////////////////////////////////
		// Not yet fully running:
		const FString StartIndex = FString::Printf(TEXT("#%3d"), StartedServiceClasses.IndexOfByKey(ServiceClass));
		if (!GameServiceManager->IsServiceRunning(ServiceClass))
		{
			StartedServicesInfo.Add(FString::Printf(TEXT("{orange}<Starting> {white}%s [{yellow}%s{white} | %s] {yellow}%s"),
				*StartIndex, *GetNameSafe(ServiceClass), *GetNameSafe(InstanceClass),
				*(IsValid(ServiceInstance) ? ServiceInstance->GetServiceStatusInfo().Get(FString()) : FString())));
			StartedServicesInfo += CollectServiceDependenciesInfo(ServiceClass);
		}

		//////////////////////////////////////////////////////////
		// Started and running:
		else
		{
			StartedServicesInfo.Add(FString::Printf(TEXT("{green}<Running> {white}%s [{yellow}%s{white} | %s] {yellow}%s"),
				*StartIndex, *GetNameSafe(ServiceClass), *GetNameSafe(InstanceClass),
				*(IsValid(ServiceInstance) ? ServiceInstance->GetServiceStatusInfo().Get(FString()) : FString())));
			StartedServicesInfo += CollectServiceDependenciesInfo(ServiceClass);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	// Print collected information:
	AddTextLine("");
	AddTextLine(FString::Printf(TEXT("{white}({cyan}CRTL + D{white}) Show Dependencies [%s]"), ShouldShowDependencies() ? TEXT("{green}ON") : TEXT("{grey}OFF")));
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

TArray<FString> FGameplayDebuggerCategory_GameServices::CollectServiceDependenciesInfo(const FGameServiceClass& ServiceClass) const
{
	if (!ShouldShowDependencies())
		return {};

	TOptional<FGameServiceInstanceClass> InstanceClass = UGameServiceManager::Get().FindRegisteredServiceInstanceClass(ServiceClass);
	if (!InstanceClass.IsSet())
		return {"{white}\t+ Dependencies: {red}???"};

	TArray<FString> DependenciesInfo;
	const UGameServiceBase* ServiceClassCdo = InstanceClass.GetValue()->GetDefaultObject<UGameServiceBase>();

	if (const TArray<FGameServiceClass>& Dependencies = ServiceClassCdo->GetServiceClassDependencies(); Dependencies.Num() > 0)
	{
		DependenciesInfo.Add_GetRef("{white}\t+ Depends on Services: ")
			+= FString::JoinBy(Dependencies, TEXT("{white}, "), [](const FGameServiceClass& Dependency) {
				return FString::Printf(TEXT("{%s}%s"), *BoolToCyanOrOrange(UGameServiceManager::Get().IsServiceRunning(Dependency)), *GetNameSafe(Dependency));
			});
	}
	if (const TArray<TSubclassOf<USubsystem>>& Dependencies = ServiceClassCdo->GetSubsystemClassDependencies(); Dependencies.Num() > 0)
	{
		DependenciesInfo.Add_GetRef("{white}\t+ Depends on Subsystems: ")
			+= FString::JoinBy(Dependencies, TEXT("{white}, "), [](const TSubclassOf<USubsystem>& Dependency) {
				TArray<UObject*> DependencyInstances; GetObjectsOfClass(Dependency, OUT DependencyInstances);
				return FString::Printf(TEXT("{%s}%s"), *BoolToCyanOrOrange(DependencyInstances.Num() > 0), *GetNameSafe(Dependency));
			});
	}
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

bool FGameplayDebuggerCategory_GameServices::ShouldShowDependencies() const
{
	return CVar_GameServicesDebugger_ShowDependencies->GetBool();
}

FString FGameplayDebuggerCategory_GameServices::BoolToCyanOrOrange(bool bUseCyanOverOrange)
{
	return (bUseCyanOverOrange ? FColor::Cyan.ToString() : FColor::Orange.ToString());
}

#endif