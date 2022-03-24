// (c) by Benjamin Barz

#if WITH_GAMEPLAY_DEBUGGER

#include "ServiceLocator/Debug/GameplayDebuggerCategory_ServiceLocator.h"

FGameplayDebuggerCategory_ServiceLocator::FGameplayDebuggerCategory_ServiceLocator()
{
	bShowOnlyWithDebugActor = false;
}

void FGameplayDebuggerCategory_ServiceLocator::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	FGameplayDebuggerCategory::CollectData(OwnerPC, DebugActor);
	if (!OwnerPC)
		return;

	if (!ServiceLocator.IsValid())
	{
		ServiceLocator = UServiceLocatorSubsystem::Get(OwnerPC->GetGameInstance());
	}
	if (ServiceLocator.IsValid())
	{
		CollectServiceLocatorData();
	}
}

void FGameplayDebuggerCategory_ServiceLocator::CollectServiceLocatorData()
{
	AddTextLine("{white}Registered Services");
	AddTextLine("{white}-------------------");

	for (const auto& Itr : ServiceLocator->RegisteredServicesForClasses)
	{
		const TSubclassOf<UInterface>& ServiceClass = Itr.Key;
		const TWeakObjectPtr<UObject>& RegisteredObject = Itr.Value;
		const FColor ObjectTextColor = (RegisteredObject.IsValid() ? FColor::Green : FColor::Red);

		AddTextLine(FString::Printf(TEXT("{white}[{yellow}I%s{white}] {%s}%s"),
			*GetNameSafe(ServiceClass),
			*ObjectTextColor.ToString(),
			*GetNameSafe(RegisteredObject.Get())));

		if (RegisteredObject.IsValid())
		{
			AddTextLine(FString::Printf(TEXT("\t{grey}-> %s"),
				*GetPathNameSafe(RegisteredObject.Get())));
		}
	}
}

#endif
