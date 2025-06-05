///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendScenario UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Scenario/Scenario.h"

#include "Algo/Count.h"
#include "Engine/LatentActionManager.h"
#include "GameplayTask.h"
#include "LatentActions.h"

namespace
{
	class FLatentScenarioActionBase : public FPendingLatentAction
	{
	public:
		TWeakObjectPtr<const UScenario> ScenarioPtr;
		FName ExecutionFunction;
		int32 OutputLink;
		FWeakObjectPtr CallbackTarget;
		bool bWantsToBeDone = false;

		FLatentScenarioActionBase(const UScenario& Scenario, const FLatentActionInfo& LatentInfo) :
			ScenarioPtr(MakeWeakObjectPtr(&Scenario)),
			ExecutionFunction(LatentInfo.ExecutionFunction),
			OutputLink(LatentInfo.Linkage),
			CallbackTarget(LatentInfo.CallbackTarget)
		{}

		virtual FString GetScenarioDescription(const UScenario& Scenario) const { return Scenario.GetInstanceName().ToString(); };
		virtual void UpdateOperation(FLatentResponse& Response, const UScenario& Scenario) = 0;

	private:
		virtual void UpdateOperation(FLatentResponse& Response) override
		{
			if (bWantsToBeDone || !ScenarioPtr.IsValid())
			{
				Response.DoneIf(true);
				return;
			}
			UpdateOperation(Response, *ScenarioPtr);
		}

#if WITH_EDITOR
		virtual FString GetDescription() const override { return ScenarioPtr.IsValid() ? GetScenarioDescription(*ScenarioPtr) : FString(); }
#endif
	};
}

///////////////////////////////////////////////////////////////////////////////////////

UScenario* UScenario::RunScenario(UScenario* TaskOwner, TSubclassOf<UScenario> Class, FName TaskName)
{
	return NewScenarioTask<UScenario>(*TaskOwner, Class, TaskName);
}

void UScenario::RestartAtEntryPoint(FGameplayTag EntryPoint)
{
	ReceiveCancelled();

	Cleanup();

	ReceiveStartAtEntryPoint(EntryPoint);
}

TArray<UScenario*> UScenario::GetRunningSubScenarios() const
{
	TArray<UScenario*> SubScenarios;
	Algo::TransformIf(ActiveChildTasks, OUT SubScenarios,
		[](const UGameplayTask* Task){ return Task->IsA<UScenario>(); },
		[](UGameplayTask* Task){ return Cast<UScenario>(Task); });
	return SubScenarios;
}

int32 UScenario::GetNumRunningSubScenarios() const
{
	return Algo::CountIf(ActiveChildTasks, [](const UGameplayTask* Task){ return Task->IsA<UScenario>(); });
}

void UScenario::WaitForRunningScenariosToComplete(FLatentActionInfo LatentInfo)
{
	class FLatentWaitScenarioCompletionAction : public FLatentScenarioActionBase
	{
	public:
		FLatentWaitScenarioCompletionAction(const UScenario& Scenario, const FLatentActionInfo& LatentInfo) :
			FLatentScenarioActionBase(Scenario, LatentInfo) {}

		virtual void UpdateOperation(FLatentResponse& Response, const UScenario& Scenario) override
		{
			const bool bIsDone = (!ScenarioPtr.IsValid() || ScenarioPtr->GetNumRunningSubScenarios() == 0);
			Response.FinishAndTriggerIf(bIsDone, ExecutionFunction, OutputLink, CallbackTarget);
		}

		virtual FString GetScenarioDescription(const UScenario& Scenario) const override
		{
			const FString ScenariosString = FString::JoinBy(Scenario.GetRunningSubScenarios(), TEXT(", "), [](const UScenario* Sc){ return Sc->GetInstanceName().ToString(); });
			return ScenariosString.IsEmpty() ? "" : "Waiting for " + ScenariosString;
		}
	};

	FLatentActionManager& LatentActionManager = GetWorld()->GetLatentActionManager();
	if (auto* LatentAction = LatentActionManager.FindExistingAction<FLatentWaitScenarioCompletionAction>(LatentInfo.CallbackTarget, LatentInfo.UUID))
	{
		LatentAction->ScenarioPtr.Reset();
	}
	LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, new FLatentWaitScenarioCompletionAction(*this, LatentInfo));
}
