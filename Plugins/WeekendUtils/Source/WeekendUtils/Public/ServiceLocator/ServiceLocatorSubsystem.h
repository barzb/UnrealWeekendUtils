// (c) by Benjamin Barz

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "ServiceLocatorSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnServiceRegistrationEvent, TSubclassOf<UInterface>, ServiceClass, UObject*, ServiceObject);

UCLASS(Blueprintable)
class WEEKENDUTILS_API UServiceLocatorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	friend class FGameplayDebuggerCategory_ServiceLocator;

public:
	// - UGameInstanceSubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// --

	static UServiceLocatorSubsystem* Get(const UGameInstance* GameInstance);

	template<class T>
	void RegisterService(UObject* ServiceObject)
	{
		static_assert(TIsIInterface<T>::Value, "Type is not an interface.");
		RegisterService(T::UClassType::StaticClass(), ServiceObject);
	}

	template<class T>
	void WithdrawService(UObject* ServiceObject)
	{
		static_assert(TIsIInterface<T>::Value, "Type is not an interface.");
		WithdrawService(T::UClassType::StaticClass(), ServiceObject);
	}

	template<class T>
	bool IsServiceRegistered() const
	{
		static_assert(TIsIInterface<T>::Value, "Type is not an interface.");
		return IsServiceOfClassRegistered(T::UClassType::StaticClass());
	}

	template<class T>
	TScriptInterface<T> GetService() const
	{
		static_assert(TIsIInterface<T>::Value, "Type is not an interface.");
		return TScriptInterface<T>(GetService(T::UClassType::StaticClass()));
	}

	UFUNCTION(BlueprintCallable, Category="Services")
	void RegisterService(TSubclassOf<UInterface> ServiceClass, UObject* ServiceObject);

	UFUNCTION(BlueprintCallable, Category="Services")
	void WithdrawService(TSubclassOf<UInterface> ServiceClass, UObject* ServiceObject);

	UFUNCTION(BlueprintPure, Category="Services")
	bool IsServiceOfClassRegistered(TSubclassOf<UInterface> ServiceClass) const;

	UFUNCTION(BlueprintCallable, Category="Services")
	UObject* GetService(TSubclassOf<UInterface> ServiceClass) const;

	UPROPERTY(BlueprintAssignable)
	FOnServiceRegistrationEvent OnServiceRegistered;

	UPROPERTY(BlueprintAssignable)
	FOnServiceRegistrationEvent OnServiceWithdrew;

private:
	UPROPERTY()
	TMap<TSubclassOf<UInterface>, TWeakObjectPtr<UObject>> RegisteredServicesForClasses;
};


template<class T>
static TScriptInterface<T> GetService(const UGameInstance* GameInstance)
{
	UServiceLocatorSubsystem* ServiceLocator = UServiceLocatorSubsystem::Get(GameInstance);
	if (!ServiceLocator)
		return nullptr;

	static_assert(TIsIInterface<T>::Value, "Type is not an interface.");
	return TScriptInterface<T>(ServiceLocator->GetService(T::UClassType::StaticClass()));
}