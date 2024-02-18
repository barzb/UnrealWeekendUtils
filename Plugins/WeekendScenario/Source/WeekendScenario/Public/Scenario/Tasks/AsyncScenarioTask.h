///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2024 by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendScenario UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameService/GameServiceUser.h"
#include "GameplayTagContainer.h"
#include "GameplayTask.h"

#include "AsyncScenarioTask.generated.h"

/**
 * #todo-docs
 */
UCLASS(Abstract)
class WEEKENDSCENARIO_API UAsyncScenarioTask : public UGameplayTask,
											   public FGameServiceUser
{
	GENERATED_BODY()

public:
	///////////////////////////////////////////////////////////////////////////////////////
	/// EVENTS

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCompleted, FGameplayTag, Result);
	UPROPERTY(BlueprintAssignable)
	FOnCompleted OnCompleted;

	///////////////////////////////////////////////////////////////////////////////////////

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsRunning() const { return IsActive(); }

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool HasCompleted() const { return IsFinished(); }

	UFUNCTION(BlueprintPure)
	FORCEINLINE FText GetDisplayName() const { return DisplayName; }

	TOptional<FString> GetDebugStringForChildTask(FName ChildTaskInstanceName) const;
	TOptional<FString> GetDebugStringForChildTask(const UAsyncScenarioTask& ChildTaskRef) const;

	double GetRuntime() const;

	///////////////////////////////////////////////////////////////////////////////////////
	/// OVERRIDES

	// - UGameplayTask
	virtual FString GetDebugString() const override;
	virtual UGameplayTasksComponent* GetGameplayTasksComponent(const UGameplayTask& Task) const override;
	virtual AActor* GetGameplayTaskOwner(const UGameplayTask* Task) const override;
	virtual void OnGameplayTaskInitialized(UGameplayTask& Task) override;
	virtual void OnGameplayTaskActivated(UGameplayTask& Task) override;
	virtual void OnGameplayTaskDeactivated(UGameplayTask& Task) override;
	// --

	///////////////////////////////////////////////////////////////////////////////////////

	template <class T>
	static T* NewScenarioTask(UAsyncScenarioTask& TaskOwner, const TSubclassOf<UAsyncScenarioTask>& TaskClass, FName TaskName)
	{
		return NewScenarioTask<T>(TScriptInterface<IGameplayTaskOwnerInterface>(&TaskOwner), TaskClass, TaskName);
	}

	template <class T>
	static T* NewScenarioTask(UAsyncScenarioTask& TaskOwner, FName TaskName)
	{
		return NewScenarioTask<T>(TScriptInterface<IGameplayTaskOwnerInterface>(&TaskOwner), T::StaticClass(), TaskName);
	}

	template <class T>
	static T* NewScenarioTask(const TScriptInterface<IGameplayTaskOwnerInterface>& TaskOwner, FName TaskName)
	{
		return NewScenarioTask<T>(TaskOwner, T::StaticClass(), TaskName);
	}

	template <class T>
	static T* NewScenarioTask(const TScriptInterface<IGameplayTaskOwnerInterface>& TaskOwner, const TSubclassOf<UAsyncScenarioTask>& TaskClass, FName TaskName)
	{
		check(IsValid(TaskOwner.GetObject()));
		check(TaskClass && TaskClass->IsChildOf<T>());

		T* NewTask = NewObject<T>(TaskOwner.GetObject(), TaskClass);
		NewTask->InitTask(*TaskOwner, TaskOwner->GetGameplayTaskDefaultPriority());
		NewTask->InstanceName = TaskName;

		return NewTask;
	}

protected:
	///////////////////////////////////////////////////////////////////////////////////////
	/// CLASS CONFIG

	//#todo-scenario polymorphic UI data stuff?
	UPROPERTY(EditDefaultsOnly)
	FText DisplayName = FText();

	UPROPERTY(EditDefaultsOnly, AdvancedDisplay)
	bool bKeepTaskRunningAfterCompletion = true;

	///////////////////////////////////////////////////////////////////////////////////////
	/// RUNTIME STATE

	UPROPERTY()
	TArray<TObjectPtr<UGameplayTask>> ChildTasks = {};

	UPROPERTY()
	TArray<TObjectPtr<UGameplayTask>> ActiveChildTasks = {};

	UPROPERTY()
	FGameplayTag CompletionResult = FGameplayTag();

	TOptional<double> TaskStartTime = {};
	TOptional<double> TaskEndTime = {};

	///////////////////////////////////////////////////////////////////////////////////////

	// - UGameplayTask
	virtual void Activate() override;
	virtual void OnDestroy(bool bHasOwnerFinished) override;
	// --

	virtual void Cleanup();

	///////////////////////////////////////////////////////////////////////////////////////
	/// BLUEPRINT API

	UFUNCTION(BlueprintCallable)
	void Complete(FGameplayTag Result = FGameplayTag());

	UFUNCTION(BlueprintNativeEvent, meta = (BlueprintProtected, DisplayName = "Start"))
	void ReceiveStart();

	UFUNCTION(BlueprintNativeEvent, meta = (BlueprintProtected, DisplayName = "Complete"))
	void ReceiveComplete(const FGameplayTag& Result);

private:
	friend class UK2Node_AsyncScenarioTask;
	TOptional<FGuid> BlueprintNodeThatSpawnedThisTask = {};
};
