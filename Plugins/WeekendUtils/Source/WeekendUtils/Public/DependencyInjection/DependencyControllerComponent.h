// (c) by Benjamin Barz

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/DependencyContainerConfig.h"
#include "UObject/WeakInterfacePtr.h"

#include "Interfaces/DependencyControllerInterface.h"

#include "DependencyControllerComponent.generated.h"

class UDependencyContainerConfig;

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class WEEKENDUTILS_API UDependencyControllerComponent : public UActorComponent,
														public IDependencyControllerInterface
{
	GENERATED_BODY()

public:
	UDependencyControllerComponent();

	// - IDependencyControllerInterface
	virtual void RegisterForDependencies(TWeakInterfacePtr<IDependencyControlledObjectInterface> Object) override;
	virtual void UnregisterForDependencies(TWeakInterfacePtr<IDependencyControlledObjectInterface> Object) override;
	// --

	/** Sets the DependencyContainerConfig from the outside. Must be called before BeginPlay(). */
	void SetConfig(const UDependencyContainerConfig* NewConfig);

protected:
	// - UActorComponent
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// --

	UPROPERTY(EditAnywhere)
	const UDependencyContainerConfig* Config = nullptr;

private:
	UFUNCTION()
	void HandleServiceRegistered(TSubclassOf<UInterface> ServiceClass, UObject* ServiceObject);

	void ProcessPendingDependencies();
	void ProcessPendingDependenciesForObject(IDependencyControlledObjectPtr ObjectWithDependencies);

	TArray<TWeakInterfacePtr<IDependencyControlledObjectInterface>> ObjectsWithPendingDependencies;
};
