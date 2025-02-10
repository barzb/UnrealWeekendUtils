///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendScenario UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "AsyncScenarioTaskGraphNodeFactory.h"

#include "KismetNodes/KismetNodeInfoContext.h"
#include "Scenario/Tasks/AsyncScenarioTask.h"

void FAsyncScenarioTaskGraphNodeFactory::SAsyncScenarioTaskGraphNode::Construct(const FArguments& InArgs, UK2Node_AsyncScenarioTask* InNode)
{
	SGraphNodeK2Default::Construct(SGraphNodeK2Default::FArguments(), InNode);
	TaskNode = MakeWeakObjectPtr(InNode);
}

void FAsyncScenarioTaskGraphNodeFactory::SAsyncScenarioTaskGraphNode::GetNodeInfoPopups(FNodeInfoContext* Context, TArray<FGraphInformationPopupInfo>& Popups) const
{
	SGraphNodeK2Default::GetNodeInfoPopups(Context, IN OUT Popups);

	const FKismetNodeInfoContext* K2Context = static_cast<FKismetNodeInfoContext*>(Context);
	const UAsyncScenarioTask* ActiveObject = Cast<UAsyncScenarioTask>(K2Context->ActiveObjectBeingDebugged);
	if (!ActiveObject)
		return;

	const UAsyncScenarioTask* TaskNodeSpawnedTask = TaskNode->FindSubtaskSpawnedByThisNode(*ActiveObject);
	if (!TaskNodeSpawnedTask)
		return;

	if (TOptional<FString> DebugString = ActiveObject->GetDebugStringForChildTask(*TaskNodeSpawnedTask); DebugString.IsSet())
	{
		new(Popups) FGraphInformationPopupInfo(nullptr, LatentBubbleColor, *DebugString);
	}
}
