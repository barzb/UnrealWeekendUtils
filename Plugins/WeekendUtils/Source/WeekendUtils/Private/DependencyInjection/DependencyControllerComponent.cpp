// (c) by Benjamin Barz

#include "DependencyInjection/DependencyControllerComponent.h"

#include "DependencyInjection/Data/DependencyContainerConfig.h"
#include "ServiceLocator/ServiceLocatorSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogDependencyInjection, Log, All);

UDependencyControllerComponent::UDependencyControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDependencyControllerComponent::RegisterForDependencies(TWeakInterfacePtr<IDependencyControlledObjectInterface> Object)
{
	ObjectsWithPendingDependencies.AddUnique(Object);
	ProcessPendingDependencies();
}

void UDependencyControllerComponent::UnregisterForDependencies(TWeakInterfacePtr<IDependencyControlledObjectInterface> Object)
{
	if (!ObjectsWithPendingDependencies.Contains(Object))
		return;

	ObjectsWithPendingDependencies.Remove(Object);
	ProcessPendingDependencies();
}

void UDependencyControllerComponent::SetConfig(const UDependencyContainerConfig* NewConfig)
{
	Config = NewConfig;
}

void UDependencyControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!ensure(Config))
		return;

	if (UServiceLocatorSubsystem* ServiceLocator = UServiceLocatorSubsystem::Get(GetOwner()->GetGameInstance()))
	{
		ServiceLocator->OnServiceRegistered.AddDynamic(this, &ThisClass::HandleServiceRegistered);
		ProcessPendingDependencies();
	}
}

void UDependencyControllerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UServiceLocatorSubsystem* ServiceLocator = UServiceLocatorSubsystem::Get(GetOwner()->GetGameInstance()))
	{
		ServiceLocator->OnServiceRegistered.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void UDependencyControllerComponent::HandleServiceRegistered(TSubclassOf<UInterface>, UObject*)
{
	ProcessPendingDependencies();
}

void UDependencyControllerComponent::ProcessPendingDependencies()
{
	// The size of the ObjectsWithPendingDependencies array can change inside of the loop, so we iterate over a safe copy.
	TArray<TWeakInterfacePtr<IDependencyControlledObjectInterface>> CopyOfObjectsWithPendingDependencies = ObjectsWithPendingDependencies;
	for (TWeakInterfacePtr<IDependencyControlledObjectInterface>& ObjectWithDependencies : CopyOfObjectsWithPendingDependencies)
	{
		if (!ObjectWithDependencies.IsValid())
		{
			ObjectsWithPendingDependencies.Remove(ObjectWithDependencies);
			continue;
		}
		ProcessPendingDependenciesForObject(ObjectWithDependencies.ToScriptInterface());
	}
}

void UDependencyControllerComponent::ProcessPendingDependenciesForObject(IDependencyControlledObjectPtr ObjectWithDependencies)
{
	UServiceLocatorSubsystem* ServiceLocator = UServiceLocatorSubsystem::Get(GetOwner()->GetGameInstance());
	if (!ensure(ServiceLocator))
		return;

	const FString ObjectNameForDebugging = GetNameSafe(ObjectWithDependencies.GetObject());
	UE_LOG(LogDependencyInjection, Log, TEXT("Checking pending dependencies for %s..."), *ObjectNameForDebugging);
	if (const FDependencyList* DependencyList = Config->GetDependenciesFor(ObjectWithDependencies.GetObject()))
	{
		bool bAreAllDependenciesAvailable = true;
		IDependencyControlledObjectInterface::FDependencyContainer DependencyContainer;
		for (const TSubclassOf<UObject>& DependencyClass : DependencyList->Entries)
		{
			// We expect that the DependencyClass is actually a subclass of UInterface, because
			// the FDependencyList::Entries property only allows UInterface derived classes.
			check(DependencyClass->IsChildOf(UInterface::StaticClass()));
			if (UObject* DependencyObject = ServiceLocator->GetService(TSubclassOf<UInterface>(DependencyClass)))
			{
				UE_LOG(LogDependencyInjection, Log, TEXT("\t<+> Available: %s"), *GetNameSafe(DependencyClass));
				DependencyContainer.DependencyObjectsForClasses.Add(DependencyClass, DependencyObject);
			}
			else
			{
				bAreAllDependenciesAvailable = false;
				UE_LOG(LogDependencyInjection, Log, TEXT("\t<-> Unavailable: %s"), *GetNameSafe(DependencyClass));
			}
		}

		UE_CLOG(!bAreAllDependenciesAvailable, LogDependencyInjection, Log, TEXT("\tNot all dependencies for %s are available, yet."), *ObjectNameForDebugging);
		if (bAreAllDependenciesAvailable)
		{
			UE_LOG(LogDependencyInjection, Log, TEXT("\t= All dependencies available. Begin injection."));
			ObjectWithDependencies->InjectDependencies(DependencyContainer);
			ObjectsWithPendingDependencies.Remove(ObjectWithDependencies.GetObject());
		}
	}
	else
	{
		UE_LOG(LogDependencyInjection, Error, TEXT("There is no DependencyContainerConfig entry for %s in %s"), *ObjectNameForDebugging, *GetNameSafe(Config));
		ensureAlways(false);
		ObjectsWithPendingDependencies.Remove(ObjectWithDependencies.GetObject());
	}
}
