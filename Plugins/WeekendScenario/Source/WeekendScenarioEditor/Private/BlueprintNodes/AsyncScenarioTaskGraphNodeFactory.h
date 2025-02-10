///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendScenario UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "EdGraph/EdGraphNode.h"
#include "EdGraphUtilities.h"
#include "K2Node_AsyncScenarioTask.h"
#include "KismetNodes/SGraphNodeK2Default.h"

class FAsyncScenarioTaskGraphNodeFactory : public FGraphPanelNodeFactory
{
protected:
	class SAsyncScenarioTaskGraphNode : public SGraphNodeK2Default
	{
	public:
		SLATE_BEGIN_ARGS(SAsyncScenarioTaskGraphNode) {}
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, UK2Node_AsyncScenarioTask* InNode);
		virtual void GetNodeInfoPopups(FNodeInfoContext* Context, TArray<FGraphInformationPopupInfo>& Popups) const override;

	protected:
		TWeakObjectPtr<UK2Node_AsyncScenarioTask> TaskNode = nullptr;
	};

	virtual TSharedPtr<SGraphNode> CreateNode(UEdGraphNode* InNode) const override
	{
		if (UK2Node_AsyncScenarioTask* ScenarioNode = Cast<UK2Node_AsyncScenarioTask>(InNode))
		{
			return SNew(SAsyncScenarioTaskGraphNode, ScenarioNode);
		}
		return nullptr;
	}
};
