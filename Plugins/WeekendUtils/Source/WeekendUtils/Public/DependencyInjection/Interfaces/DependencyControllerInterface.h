// (c) by Benjamin Barz

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/WeakInterfacePtr.h"

#include "DependencyControlledObjectInterface.h"

#include "DependencyControllerInterface.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UDependencyControllerInterface : public UInterface
{
	GENERATED_BODY()
};

class WEEKENDUTILS_API IDependencyControllerInterface
{
	GENERATED_BODY()

public:
	virtual void RegisterForDependencies(TWeakInterfacePtr<IDependencyControlledObjectInterface> Object) = 0;
	virtual void UnregisterForDependencies(TWeakInterfacePtr<IDependencyControlledObjectInterface> Object) = 0;
};

using IDependencyControllerPtr = TScriptInterface<IDependencyControllerInterface>;