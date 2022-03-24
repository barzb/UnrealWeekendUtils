// (c) by Benjamin Barz

#pragma once

#if WITH_GAMEPLAY_DEBUGGER

#include "CoreMinimal.h"
#include "GameplayDebuggerCategory.h"
#include "GameplayDebugger/GameplayDebuggerUtils.h"
#include "ServiceLocator/ServiceLocatorSubsystem.h"

class WEEKENDUTILS_API FGameplayDebuggerCategory_ServiceLocator : public FGameplayDebuggerCategory
{
public:
	FGameplayDebuggerCategory_ServiceLocator();
	GENERATE_DEBUGGER_CATEGORY(ServiceLocator);

	// - FGameplayDebuggerCategory
	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	// --

private:
	void CollectServiceLocatorData();

	TWeakObjectPtr<UServiceLocatorSubsystem> ServiceLocator;
};

#endif // WITH_GAMEPLAY_DEBUGGER