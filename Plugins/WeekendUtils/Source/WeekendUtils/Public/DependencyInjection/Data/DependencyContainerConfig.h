// (c) by Benjamin Barz

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "DependencyContainerConfig.generated.h"

USTRUCT(BlueprintType)
struct WEEKENDUTILS_API FDependencyList
{
	GENERATED_BODY()

	friend class UDependencyContainerConfig;

public:
	/**
	 * List of classes that something can depend on. Must be Interfaces!
	 * UInterfaces must be BlueprintType to show up in this dropdown.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowAbstract = true, AllowedClasses = "UInterface", ExactClass = false))
	TArray<TSubclassOf<UObject>> Entries;

private:
	template <typename T>
	FDependencyList& DependsOn()
	{
		Entries.Add(T::UClassType::StaticClass());
		return *this;
	}
};

UCLASS(Abstract, Blueprintable, BlueprintType)
class WEEKENDUTILS_API UDependencyContainerConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	const FDependencyList* GetDependenciesFor(const UObject* ObjectWithDependencies) const
	{
		for (const auto& ConfigEntry : DependenciesForClasses)
		{
			const UClass* ClassWithDependencies = ConfigEntry.Key;
			const FDependencyList& Dependencies = ConfigEntry.Value;

			const UClass* ClassOfObjectWithDependencies = ObjectWithDependencies->GetClass();
			if (ClassOfObjectWithDependencies->IsChildOf(ClassWithDependencies) ||
				ClassOfObjectWithDependencies->ImplementsInterface(ClassWithDependencies))
				return &Dependencies;
		}
		return nullptr;
	}

	/**
	 * List of object classes that depend on certain systems.
	 * Key: Object class that has dependencies. All classes allowed.
	 * Value: List of interface classes the systems the Key class depends on.
	 * The dependencies are injected as soon as all services for the respective
	 * interfaces are registered with the ServiceLocatorSubsystem.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowAbstract = true, AllowedClasses = "UInterface, UObject", ExactClass = false))
	TMap<TSubclassOf<UObject>, FDependencyList> DependenciesForClasses;

protected:
	template <typename T>
	typename TEnableIf<TIsDerivedFrom<T, UObject>::IsDerived, FDependencyList&>::Type Add()
	{
		return DependenciesForClasses.Add(T::StaticClass());
	}

	template <typename T>
	typename TEnableIf<TIsIInterface<T>::Value, FDependencyList&>::Type Add()
	{
		return DependenciesForClasses.Add(T::UClassType::StaticClass());
	}

	// - UPrimaryDataAsset
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
	// --
};
