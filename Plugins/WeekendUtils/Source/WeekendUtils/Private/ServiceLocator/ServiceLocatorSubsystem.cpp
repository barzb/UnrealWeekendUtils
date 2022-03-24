// (c) by Benjamin Barz

#include "ServiceLocator/ServiceLocatorSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogServiceLocator, Log, All);

UServiceLocatorSubsystem* UServiceLocatorSubsystem::Get(const UGameInstance* GameInstance)
{
	return UGameInstance::GetSubsystem<UServiceLocatorSubsystem>(GameInstance);
}

void UServiceLocatorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RegisteredServicesForClasses.Empty();

	UE_LOG(LogServiceLocator, Log, TEXT("ServiceLocatorSubsystem initialized."));
}

void UServiceLocatorSubsystem::Deinitialize()
{
	RegisteredServicesForClasses.Empty();
	Super::Deinitialize();
}

void UServiceLocatorSubsystem::RegisterService(TSubclassOf<UInterface> ServiceClass, UObject* ServiceObject)
{
	if (IsServiceOfClassRegistered(ServiceClass))
	{
		ensureAlwaysMsgf(false, TEXT("RegisterService: Service of class %s is already registered at ServiceLocatorSubsystem."), *GetNameSafe(ServiceClass));
		return;
	}

	RegisteredServicesForClasses.Add(ServiceClass, MakeWeakObjectPtr(ServiceObject));
	OnServiceRegistered.Broadcast(ServiceClass, ServiceObject);

	UE_LOG(LogServiceLocator, Log, TEXT("Service<I%s> registered: %s"), *GetNameSafe(ServiceClass), *GetPathNameSafe(ServiceObject));
}

void UServiceLocatorSubsystem::WithdrawService(TSubclassOf<UInterface> ServiceClass, UObject* ServiceObject)
{
	if (!IsServiceOfClassRegistered(ServiceClass))
	{
		ensureAlwaysMsgf(false, TEXT("WithdrawService: Service of class I%s is not registered at ServiceLocatorSubsystem."), *GetNameSafe(ServiceClass));
		return;
	}

	RegisteredServicesForClasses.Remove(ServiceClass);
	OnServiceWithdrew.Broadcast(ServiceClass, ServiceObject);

	UE_LOG(LogServiceLocator, Log, TEXT("Service<I%s> withdrawn: %s"), *GetNameSafe(ServiceClass), *GetPathNameSafe(ServiceObject));
}

bool UServiceLocatorSubsystem::IsServiceOfClassRegistered(TSubclassOf<UInterface> ServiceClass) const
{
	return RegisteredServicesForClasses.Contains(ServiceClass);
}

UObject* UServiceLocatorSubsystem::GetService(TSubclassOf<UInterface> ServiceClass) const
{
	if (!IsServiceOfClassRegistered(ServiceClass))
		return nullptr;

	if (!RegisteredServicesForClasses[ServiceClass].IsValid())
		return nullptr;

	return RegisteredServicesForClasses[ServiceClass].Get();
}
