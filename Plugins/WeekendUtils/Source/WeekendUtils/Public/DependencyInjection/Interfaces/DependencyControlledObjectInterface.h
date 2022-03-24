// (c) by Benjamin Barz

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "DependencyControlledObjectInterface.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UDependencyControlledObjectInterface : public UInterface
{
	GENERATED_BODY()
};

class WEEKENDUTILS_API IDependencyControlledObjectInterface
{
	GENERATED_BODY()

public:
	struct FDependencyContainer
	{
		FDependencyContainer() = default;

		~FDependencyContainer()
		{
			DependencyObjectsForClasses.Empty();
		}

		template <typename T>
		typename TEnableIf<TIsDerivedFrom<T, UObject>::IsDerived, T*>::Type Get() const
		{
			if (!DependencyObjectsForClasses.Contains(T::StaticClass()))
				return nullptr;

			return DependencyObjectsForClasses[T::StaticClass()].Get();
		}

		template <typename T>
		typename TEnableIf<TIsIInterface<T>::Value, TScriptInterface<T>>::Type Get() const
		{
			if (!DependencyObjectsForClasses.Contains(T::UClassType::StaticClass()))
				return nullptr;

			return TScriptInterface<T>(DependencyObjectsForClasses[T::UClassType::StaticClass()].Get());
		}

		TMap<UClass*, TWeakObjectPtr<UObject>> DependencyObjectsForClasses;
	};

	virtual void InjectDependencies(FDependencyContainer DependencyContainer) = 0;
};

using IDependencyControlledObjectPtr = TScriptInterface<IDependencyControlledObjectInterface>;