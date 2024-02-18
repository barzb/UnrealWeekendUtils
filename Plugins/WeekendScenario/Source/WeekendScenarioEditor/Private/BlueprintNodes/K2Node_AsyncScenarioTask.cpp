///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2024 by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendScenario UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "K2Node_AsyncScenarioTask.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintFunctionNodeSpawner.h"
#include "GameplayTagsK2Node_SwitchGameplayTag.h"
#include "K2Node_AddDelegate.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_ExecutionSequence.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_TemporaryVariable.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "KismetCompiler.h"
#include "Scenario/Tasks/AsyncScenarioTask.h"

namespace
{
	FName PN_ScenarioClass = TEXT("Class");
	FName PN_TaskName = TEXT("TaskName");
	FName PN_ExecOutDefault = TEXT("Other Results");
	FName PN_OnCompletedResult = TEXT("Result");
}

UK2Node_AsyncScenarioTask::UK2Node_AsyncScenarioTask(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		RegisterSpecializedTaskNodeClass(GetClass());
	}
}

FName UK2Node_AsyncScenarioTask::GetScenarioTaskClassPinName() const
{
	return PN_ScenarioClass;
}

FName UK2Node_AsyncScenarioTask::GetScenarioTaskNamePinName() const
{
	return PN_TaskName;
}

UEdGraphPin* UK2Node_AsyncScenarioTask::GetScenarioTaskClassPin(const TArray<UEdGraphPin*>* InPinsToSearch) const
{
	const TArray<UEdGraphPin*>* PinsToSearch = InPinsToSearch ? InPinsToSearch : &Pins;
	for (UEdGraphPin* Pin : *PinsToSearch)
	{
		if (Pin && Pin->PinName == GetScenarioTaskClassPinName() && Pin->Direction == EGPD_Input)
			return Pin;
	}

	return nullptr;
}

UClass* UK2Node_AsyncScenarioTask::GetScenarioTaskClassToSpawn(const TArray<UEdGraphPin*>* InPinsToSearch) const
{
	UClass* ScenarioTaskClass = nullptr;
	const TArray<UEdGraphPin*>* PinsToSearch = InPinsToSearch ? InPinsToSearch : &Pins;

	UEdGraphPin* TaskClassInputPin = GetScenarioTaskClassPin(PinsToSearch);
	if (TaskClassInputPin && TaskClassInputPin->DefaultObject != nullptr && TaskClassInputPin->LinkedTo.Num() == 0)
	{
		ScenarioTaskClass = CastChecked<UClass>(TaskClassInputPin->DefaultObject);
	}
	else if (TaskClassInputPin && (1 == TaskClassInputPin->LinkedTo.Num()))
	{
		const UEdGraphPin* SourcePin = TaskClassInputPin->LinkedTo[0];
		ScenarioTaskClass = SourcePin ? Cast<UClass>(SourcePin->PinType.PinSubCategoryObject.Get()) : nullptr;
	}

	return ScenarioTaskClass;
}

TOptional<FName> UK2Node_AsyncScenarioTask::GetScenarioTaskInstanceName() const
{
	if (const UEdGraphPin* InstanceNamePin = FindPin(PN_TaskName, EGPD_Input))
		return FName(InstanceNamePin->DefaultValue);

	return {};
}

bool UK2Node_AsyncScenarioTask::HasScenarioTaskClassPin() const
{
	return (GetScenarioTaskClassPin() != nullptr);
}

bool UK2Node_AsyncScenarioTask::HasCompletionResult() const
{
	const UFunction* ProxyFunction = ProxyFactoryClass->FindFunctionByName(ProxyFactoryFunctionName);
	return (ProxyFunction && (ProxyFunction->HasMetaData(TEXT("HideResult")) == false));
}

bool UK2Node_AsyncScenarioTask::ShouldBranchResultTag() const
{
	return (bCanBranchResultTag && bBranchResultTag);
}

UAsyncScenarioTask* UK2Node_AsyncScenarioTask::FindSubtaskSpawnedByThisNode(const UAsyncScenarioTask& OwningTask) const
{
	for (UGameplayTask* Task : OwningTask.ChildTasks)
	{
		UAsyncScenarioTask* ScenarioTask = Cast<UAsyncScenarioTask>(Task);
		if (IsValid(ScenarioTask) && ScenarioTask->BlueprintNodeThatSpawnedThisTask.Get(FGuid()) == NodeGuid)
			return ScenarioTask;
	}

	return nullptr;
}

void UK2Node_AsyncScenarioTask::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	struct FGetMenuActions_Utils
	{
		static void SetNodeFunc(UEdGraphNode* NewNode, bool /*bIsTemplateNode*/, TWeakObjectPtr<UFunction> FunctionPtr)
		{
			UK2Node_AsyncScenarioTask* AsyncTaskNode = CastChecked<UK2Node_AsyncScenarioTask>(NewNode);
			if (FunctionPtr.IsValid())
			{
				const UFunction* Func = FunctionPtr.Get();
				const FObjectProperty* ReturnProp = CastFieldChecked<FObjectProperty>(Func->GetReturnProperty());

				AsyncTaskNode->ProxyFactoryFunctionName = Func->GetFName();
				AsyncTaskNode->ProxyFactoryClass = Func->GetOuterUClass();
				AsyncTaskNode->ProxyClass = ReturnProp->PropertyClass;
			}
		}
	};

	ActionRegistrar.RegisterClassFactoryActions<UAsyncScenarioTask>(
		FBlueprintActionDatabaseRegistrar::FMakeFuncSpawnerDelegate::CreateLambda([NodeClass = GetClass()](const UFunction* FactoryFunc)
		{
			UBlueprintNodeSpawner* NodeSpawner = UBlueprintFunctionNodeSpawner::Create(FactoryFunc);
			check(NodeSpawner != nullptr);
			NodeSpawner->NodeClass = NodeClass;

			TWeakObjectPtr<UFunction> FunctionPtr = MakeWeakObjectPtr(const_cast<UFunction*>(FactoryFunc));
			NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(FGetMenuActions_Utils::SetNodeFunc, FunctionPtr);

			return NodeSpawner;
		}));
}

FText UK2Node_AsyncScenarioTask::GetTooltipText() const
{
	const UClass* ScenarioTaskClass = GetScenarioTaskClassToSpawn();
	return FText::Format(INVTEXT("Runs an async scenario task that can complete at a later time.\n\nDouble-click this node to open: {0}."),
		FText::FromString(GetNameSafe(ScenarioTaskClass)));
}

FText UK2Node_AsyncScenarioTask::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	FText Title = Super::GetNodeTitle(TitleType);
	if (const UClass* ScenarioTaskClass = GetScenarioTaskClassToSpawn())
	{
		FText DisplayName = ScenarioTaskClass->GetDefaultObject<UAsyncScenarioTask>()->GetDisplayName();
		if (DisplayName.IsEmpty())
		{
			DisplayName = ScenarioTaskClass->GetDisplayNameText();
		}
		Title = FText::Format(INVTEXT("{0}: {1}"), Title, DisplayName);
	}
	return Title;
}

UObject* UK2Node_AsyncScenarioTask::GetJumpTargetForDoubleClick() const
{
	if (const UClass* ScenarioTaskClass = GetScenarioTaskClassToSpawn())
	{
		return UBlueprint::GetBlueprintFromClass(ScenarioTaskClass);
	}
	return Super::GetJumpTargetForDoubleClick();
}

bool UK2Node_AsyncScenarioTask::IsCompatibleWithGraph(UEdGraph const* TargetGraph) const
{
	const EGraphType GraphType = TargetGraph->GetSchema()->GetGraphType(TargetGraph);
	if (GraphType == GT_Ubergraph || GraphType == GT_Macro)
	{
		const UBlueprint* MyBlueprint = FBlueprintEditorUtils::FindBlueprintForGraph(TargetGraph);
		if (MyBlueprint && MyBlueprint->GeneratedClass)
		{
			return MyBlueprint->GeneratedClass->IsChildOf(UAsyncScenarioTask::StaticClass());
		}
	}

	return false;
}

void UK2Node_AsyncScenarioTask::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
	Super::ValidateNodeDuringCompilation(MessageLog);

	if (const UClass* ScenarioTaskClass = GetScenarioTaskClassToSpawn(); HasScenarioTaskClassPin() && !IsValid(ScenarioTaskClass))
	{
		MessageLog.Error(TEXT("@@ is missing parameter: @@"), this, GetScenarioTaskClassPin());
	}

	if (TOptional<FName> TaskInstanceName = GetScenarioTaskInstanceName(); TaskInstanceName.IsSet() && TaskInstanceName->IsNone())
	{
		//MessageLog.Warning(TEXT("@@ is missing parameter: @@"), this, FindPin(GetScenarioTaskNamePinName(), EGPD_Input));
	}

	// ...
}

bool UK2Node_AsyncScenarioTask::IsHandling(TSubclassOf<UGameplayTask> TaskClass) const
{
	return TaskClass && TaskClass->IsChildOf(UAsyncScenarioTask::StaticClass());
}

void UK2Node_AsyncScenarioTask::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
	AllocateDefaultPins();
	if (UClass* ScenarioTaskClass = GetScenarioTaskClassToSpawn(&OldPins))
	{
		CreatePinsForClass(ScenarioTaskClass);
	}
	RestoreSplitPins(OldPins);
}

void UK2Node_AsyncScenarioTask::PinDefaultValueChanged(UEdGraphPin* ChangedPin)
{
	if (ChangedPin->PinName == GetScenarioTaskClassPinName())
	{
		// Track removed pins so that we can reconnect it later if possible
		TArray<UEdGraphPin*> RemovedPins = {};

		// Orphan all pins related to archetype variables that have connections, otherwise just remove them
		for (const FName& OldPinReference : SpawnParamPins)
		{
			if (UEdGraphPin* OldPin = FindPin(OldPinReference))
			{
				if (OldPin->HasAnyConnections())
				{
					RemovedPins.Add(OldPin);
				}
				Pins.Remove(OldPin);
			}
		}

		SpawnParamPins.Reset();

		if (UClass* ScenarioTaskClass = GetScenarioTaskClassToSpawn())
		{
			CreatePinsForClass(ScenarioTaskClass);

			// Change output proxy pin to reflect the class of the scenario task input pin:
			if (UEdGraphPin* ProxyPin = FindPin(FBaseAsyncTaskHelper::GetAsyncTaskProxyName(), EGPD_Output))
			{
				ProxyPin->PinType = FEdGraphPinType(UEdGraphSchema_K2::PC_Object, NAME_None,
					ScenarioTaskClass, ChangedPin->PinType.ContainerType, false, {});
				PinTypeChanged(ProxyPin);
			}

			if (UEdGraphPin* InstanceNamePin = FindPin(PN_TaskName, EGPD_Input))
			{
				if (FName(InstanceNamePin->DefaultValue) == NAME_None)
				{
					InstanceNamePin->DefaultValue = ScenarioTaskClass->GetDisplayNameText().ToString();
					InstanceNamePin->DefaultValue.RemoveFromEnd(TEXT("_C"));
				}
			}
		}

		RewireOldPinsToNewPins(RemovedPins, Pins, nullptr);

		// Refresh the UI for the graph so the pin changes show up
		GetGraph()->NotifyNodeChanged(this);
		FBlueprintEditorUtils::MarkBlueprintAsModified(GetBlueprint());
	}
}

void UK2Node_AsyncScenarioTask::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, ResultTags) ||
		PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, bBranchResultTag) ||
		PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, bShortenResultPins) ||
		PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, bAddBranchForOtherResults))
	{
		ReconstructNode();
		Super::PostEditChangeProperty(PropertyChangedEvent);
		GetGraph()->NotifyNodeChanged(this);
	}
	else
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);
	}
}

void UK2Node_AsyncScenarioTask::AllocateDefaultPins()
{
	bCanBranchResultTag = HasCompletionResult();
	if (ShouldBranchResultTag() && ResultTags.Num() > 0)
	{
		for (const FGameplayTag& ResultTag : ResultTags)
		{
			UEdGraphPin* Pin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, ResultTag.GetTagName());
			if (bShortenResultPins)
			{
				const FGameplayTag& ParentTag = ResultTag.RequestDirectParent();
				const int32 ParentTagLength = ParentTag.IsValid() ? (GetNum(ParentTag.ToString()) + 1) : 0;
				Pin->PinFriendlyName = FText::FromString(ResultTag.ToString().RightChop(ParentTagLength));
			}
		}
		if (bAddBranchForOtherResults)
		{
			CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, PN_ExecOutDefault);
		}
	}

	Super::AllocateDefaultPins();

	if (UEdGraphPin* ThenPin = FindPin(UEdGraphSchema_K2::PN_Then, EGPD_Output))
	{
		RemovePin(ThenPin);
	}

	if (UEdGraphPin* OnCompletedExecOutPin = FindPin(GET_MEMBER_NAME_CHECKED(UAsyncScenarioTask, OnCompleted), EGPD_Output))
	{
		if (ShouldBranchResultTag())
		{
			RemovePin(OnCompletedExecOutPin);
		}
		else if (UEdGraphPin* ProxyOutPin = FindPin(FBaseAsyncTaskHelper::GetAsyncTaskProxyName(), EGPD_Output))
		{
			const int32 ExecPinIndex = Pins.IndexOfByKey(OnCompletedExecOutPin);
			const int32 ProxyOutPinIndex = Pins.IndexOfByKey(ProxyOutPin);
			Pins.Swap(ExecPinIndex, ProxyOutPinIndex);
		}

		if (!HasCompletionResult())
		{
			RemovePin(FindPin(PN_OnCompletedResult, EGPD_Output));
		}
	}
}

void UK2Node_AsyncScenarioTask::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	const UEdGraphPin* FactoryTaskClassInputPin = GetScenarioTaskClassPin();
	if (!FactoryTaskClassInputPin)
	{
		Super::ExpandNode(CompilerContext, SourceGraph);
		return;
	}

	UK2Node::ExpandNode(CompilerContext, SourceGraph);
	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();
	check(SourceGraph && Schema);
	bool bIsErrorFree = true;

	// ------------------------------------------------------------------------------------------
	// Expand the original node by creating a new nodes that contains the original function call
	// to the factory method plus some extra nodes + pins (like the delegate outputs and expose
	// on spawn input parameters). In the end, we'll disconnect the original pin from everything.
	// ------------------------------------------------------------------------------------------
	// Create a new function call node to the original task factory method and rewire the EXEC pin
	// from the original node to the new factory method CallFunction node:
	// ------------------------------------------------------------------------------------------
	UK2Node_CallFunction* const FactoryMethodCallNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	FactoryMethodCallNode->FunctionReference.SetExternalMember(ProxyFactoryFunctionName, ProxyFactoryClass);
	FactoryMethodCallNode->AllocateDefaultPins();
	UEdGraphPin& OriginalInputExecPin = *FindPinChecked(UEdGraphSchema_K2::PN_Execute);
	UEdGraphPin& FactoryMethodInputExecPin = *FactoryMethodCallNode->FindPinChecked(UEdGraphSchema_K2::PN_Execute);
	bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(OriginalInputExecPin, FactoryMethodInputExecPin).CanSafeConnect();
	UEdGraphPin* LastThenPin = FactoryMethodCallNode->FindPinChecked(UEdGraphSchema_K2::PN_Then);

	// ------------------------------------------------------------------------------------------
	// # INPUT PINS
	// ------------------------------------------------------------------------------------------
	for (UEdGraphPin* OriginalInputPin : Pins)
	{
		if (!FBaseAsyncTaskHelper::ValidDataPin(OriginalInputPin, EGPD_Input))
			continue; // Skip output pins.

		// Copy over all input pins from the original node to the new factory CallFunction node
		// and connect them to the same nodes or values that were connected to the original node:
		if (UEdGraphPin* FactoryMethodCallInputPin = FactoryMethodCallNode->FindPin(OriginalInputPin->PinName))
		{
			bIsErrorFree &= CompilerContext.CopyPinLinksToIntermediate(*OriginalInputPin, *FactoryMethodCallInputPin).CanSafeConnect();
		}
	}
	bIsErrorFree &= ExpandDefaultToSelfPin(CompilerContext, SourceGraph, FactoryMethodCallNode);

	// Connect ExposeOnSpawn params to spawned proxy task:
	UEdGraphPin* FactoryMethodScenarioTaskOutPin = FactoryMethodCallNode->GetReturnValuePin();
	if (UClass* ScenarioTaskClass = GetScenarioTaskClassToSpawn())
	{
		bIsErrorFree &= ConnectSpawnProperties(ScenarioTaskClass, Schema, CompilerContext, SourceGraph, IN OUT LastThenPin, FactoryMethodScenarioTaskOutPin);
	}

	// Link the spawned task object to this node, mostly for the popup messages to know which node the task belongs to:
	{
		UK2Node_CallFunction* LinkToThisNodeCall = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
		LinkToThisNodeCall->FunctionReference.SetExternalMember(GET_MEMBER_NAME_CHECKED(UK2Node_AsyncScenarioTask, LinkScenarioTaskToSpawningBlueprintNode), StaticClass());
		LinkToThisNodeCall->AllocateDefaultPins();
		LinkToThisNodeCall->FindPinChecked(TEXT("OriginNodeGuid"), EGPD_Input)->DefaultValue = NodeGuid.ToString();
		UEdGraphPin* TaskInputPin = LinkToThisNodeCall->FindPinChecked(TEXT("SpawnedTask"), EGPD_Input);
		bIsErrorFree &= Schema->TryCreateConnection(FactoryMethodScenarioTaskOutPin, TaskInputPin);
		bIsErrorFree &= Schema->TryCreateConnection(LastThenPin, LinkToThisNodeCall->GetExecPin());
		LastThenPin = LinkToThisNodeCall->GetThenPin();
	}

	// Call the Activate function on the spawned proxy task:
	if (ProxyActivateFunctionName != NAME_None)
	{
		UK2Node_CallFunction* CallActivateProxyObjectNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
		CallActivateProxyObjectNode->FunctionReference.SetExternalMember(ProxyActivateFunctionName, ProxyClass);
		CallActivateProxyObjectNode->AllocateDefaultPins();
		UEdGraphPin* ActivateCallSelfPin = Schema->FindSelfPin(*CallActivateProxyObjectNode, EGPD_Input);
		bIsErrorFree &= Schema->TryCreateConnection(FactoryMethodScenarioTaskOutPin, ActivateCallSelfPin);
		bIsErrorFree &= Schema->TryCreateConnection(LastThenPin, CallActivateProxyObjectNode->GetExecPin());
		LastThenPin = CallActivateProxyObjectNode->GetThenPin();
	}

	// ------------------------------------------------------------------------------------------
	// # OUTPUT PINS 
	// ------------------------------------------------------------------------------------------
	TArray<FBaseAsyncTaskHelper::FOutputPinAndLocalVariable> OriginalOutputPinsAndTempVars;
	for (UEdGraphPin* OriginalOutputPin : Pins)
	{
		if (!FBaseAsyncTaskHelper::ValidDataPin(OriginalOutputPin, EGPD_Output))
			continue; // Skip input pins.

		if (OriginalOutputPin->GetFName() == FBaseAsyncTaskHelper::GetAsyncTaskProxyName())
		{
			// Directly rewire the connection from the "AsyncTask" output pin of the original node
			// to the new factory method CallFunction node output pin:
			bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*OriginalOutputPin, *FactoryMethodScenarioTaskOutPin).CanSafeConnect();
		}
		else
		{
			// Assign all connections to output pins of the original factory method node to a new temporary variable.
			// Since we will be waiting for the async task callback to fire, we need to cache these vars:
			const FEdGraphPinType& OriginalPinType = OriginalOutputPin->PinType;
			UK2Node_TemporaryVariable* TempVarNode = CompilerContext.SpawnInternalVariable(
				this, OriginalPinType.PinCategory, OriginalPinType.PinSubCategory, OriginalPinType.PinSubCategoryObject.Get(), OriginalPinType.ContainerType, OriginalPinType.PinValueType);
			bIsErrorFree &= TempVarNode->GetVariablePin() && CompilerContext.MovePinLinksToIntermediate(*OriginalOutputPin, *TempVarNode->GetVariablePin()).CanSafeConnect();
			OriginalOutputPinsAndTempVars.Add(FBaseAsyncTaskHelper::FOutputPinAndLocalVariable(OriginalOutputPin, TempVarNode));
		}
	}

	// ------------------------------------------------------------------------------------------
	// Insert a new IsValid and then a Branch (if/else) node after the factory method call and
	// hook them up so that the returned scenario AsyncTask object ptr is properly valid checked:
	// ------------------------------------------------------------------------------------------
	UK2Node_CallFunction* IsValidFuncNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	{
		const FName IsValidFuncName = GET_FUNCTION_NAME_CHECKED(UKismetSystemLibrary, IsValid);
		IsValidFuncNode->FunctionReference.SetExternalMember(IsValidFuncName, UKismetSystemLibrary::StaticClass());
		IsValidFuncNode->AllocateDefaultPins();
		UEdGraphPin* IsValidInputPin = IsValidFuncNode->FindPinChecked(TEXT("Object"));
		bIsErrorFree &= Schema->TryCreateConnection(FactoryMethodScenarioTaskOutPin, IsValidInputPin);
	}
	UK2Node_IfThenElse* IsValidBranchNode = CompilerContext.SpawnIntermediateNode<UK2Node_IfThenElse>(this, SourceGraph);
	{
		IsValidBranchNode->AllocateDefaultPins();
		bIsErrorFree &= Schema->TryCreateConnection(IsValidFuncNode->GetReturnValuePin(), IsValidBranchNode->GetConditionPin());
		bIsErrorFree &= Schema->TryCreateConnection(LastThenPin, IsValidBranchNode->GetExecPin());
		LastThenPin = IsValidBranchNode->GetThenPin();
	}

	// ------------------------------------------------------------------------------------------
	// Create an output EXEC pin for each DYNAMIC_MULTICAST_DELEGATE property of the scenario task:
	// ------------------------------------------------------------------------------------------
	for (TFieldIterator<FMulticastDelegateProperty> PropertyIt(ProxyClass); PropertyIt && bIsErrorFree; ++PropertyIt)
	{
		UEdGraphPin* DelegatePropertyPin = FindPin(PropertyIt->GetFName());
		bIsErrorFree &= TryExpandMulticastDelegatePropertyAsExecOutputPin(
			**PropertyIt, DelegatePropertyPin, OriginalOutputPinsAndTempVars, FactoryMethodScenarioTaskOutPin,
			IN OUT LastThenPin, SourceGraph, CompilerContext);
	}

	// --------------------------------------------------------------------------------------
	// Connect the remaining "then" pin and the else pin of the valid check to the nodes that
	// were originally linked to them (in the original event graph):
	// --------------------------------------------------------------------------------------
	if (UEdGraphPin* OriginalThenPin = FindPin(UEdGraphSchema_K2::PN_Then))
	{
		bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*OriginalThenPin, *LastThenPin).CanSafeConnect();
		bIsErrorFree &= CompilerContext.CopyPinLinksToIntermediate(*LastThenPin, *IsValidBranchNode->GetElsePin()).CanSafeConnect();
	}

	// --------------------------------------------------------------------------------------
	// # ERROR HANDLING
	// --------------------------------------------------------------------------------------
	if (!bIsErrorFree)
	{
		CompilerContext.MessageLog.Error(*INVTEXT("@@: UK2Node_AsyncScenarioTask::ExpandNode - Internal error").ToString(), this);
	}

	// --------------------------------------------------------------------------------------
	// Finally, break all node connections to and from the original node, since we substituted
	// all connections to the newly created + linked nodes:
	// --------------------------------------------------------------------------------------
	BreakAllNodeLinks();
}

bool UK2Node_AsyncScenarioTask::TryExpandMulticastDelegatePropertyAsExecOutputPin(
	FMulticastDelegateProperty& DelegateProperty, UEdGraphPin* DelegatePropertyPin, const TArray<FBaseAsyncTaskHelper::FOutputPinAndLocalVariable>& TempVarOutputs,
	UEdGraphPin* ProxyObjectPin, UEdGraphPin*& InOutLastThenPin, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext)
{
	if (DelegateProperty.GetFName() != GET_MEMBER_NAME_CHECKED(UAsyncScenarioTask, OnCompleted))
	{
		UEdGraphPin* LastActivatedThenPin = nullptr;
		return FBaseAsyncTaskHelper::HandleDelegateImplementation(
			&DelegateProperty, TempVarOutputs, ProxyObjectPin, IN OUT InOutLastThenPin, OUT LastActivatedThenPin,
			this, SourceGraph, CompilerContext);
	}

	bool bIsErrorFree = true;
	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();
	UK2Node_CustomEvent* CustomEventNode = CompilerContext.SpawnIntermediateEventNode<UK2Node_CustomEvent>(this, DelegatePropertyPin, SourceGraph);
	{
		UK2Node_AddDelegate* AddDelegateNode = CompilerContext.SpawnIntermediateNode<UK2Node_AddDelegate>(this, SourceGraph);
		AddDelegateNode->SetFromProperty(&DelegateProperty, false, DelegateProperty.GetOwnerClass());
		AddDelegateNode->AllocateDefaultPins();

		bIsErrorFree &= Schema->TryCreateConnection(AddDelegateNode->FindPinChecked(UEdGraphSchema_K2::PN_Self), ProxyObjectPin);
		bIsErrorFree &= Schema->TryCreateConnection(InOutLastThenPin, AddDelegateNode->FindPinChecked(UEdGraphSchema_K2::PN_Execute));
		InOutLastThenPin = AddDelegateNode->FindPinChecked(UEdGraphSchema_K2::PN_Then);

		CustomEventNode->CustomFunctionName = *FString::Printf(TEXT("%s_%s"), *DelegateProperty.GetName(), *CompilerContext.GetGuid(this));
		CustomEventNode->AllocateDefaultPins();

		bIsErrorFree &= FBaseAsyncTaskHelper::CopyEventSignature(CustomEventNode, AddDelegateNode->GetDelegateSignature(), Schema);
		bIsErrorFree &= Schema->TryCreateConnection(AddDelegateNode->GetDelegatePin(), CustomEventNode->FindPin(CustomEventNode->DelegateOutputName));
	}

	UEdGraphPin* LastThenPinForCustomEvent = CustomEventNode->FindPinChecked(UEdGraphSchema_K2::PN_Then);
	UEdGraphPin* OriginalDefaultOutExecPin = FindPin(PN_ExecOutDefault, EGPD_Output);
	for (const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& OriginalOutputPinAndTempVar : TempVarOutputs)
	{
		UEdGraphPin* CustomEventNodeParamOutPin = CustomEventNode->FindPin(OriginalOutputPinAndTempVar.OutputPin->PinName);
		if (CustomEventNodeParamOutPin == nullptr)
			continue;

		if (OriginalOutputPinAndTempVar.TempVar)
		{
			bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*OriginalOutputPinAndTempVar.TempVar->GetVariablePin(), *CustomEventNodeParamOutPin).CanSafeConnect();
		}

		if (UEdGraphPin* OriginalOnCompletedExecOutPin = FindPin(GET_MEMBER_NAME_CHECKED(UAsyncScenarioTask, OnCompleted), EGPD_Output))
		{
			UK2Node_ExecutionSequence* SequenceNode = CompilerContext.SpawnIntermediateNode<UK2Node_ExecutionSequence>(this, SourceGraph);
			SequenceNode->AllocateDefaultPins();
			bIsErrorFree &= Schema->TryCreateConnection(LastThenPinForCustomEvent, SequenceNode->GetExecPin());
			bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*OriginalOnCompletedExecOutPin, *SequenceNode->GetThenPinGivenIndex(0)).CanSafeConnect();
			LastThenPinForCustomEvent = SequenceNode->GetThenPinGivenIndex(1);
		}

		if (ShouldBranchResultTag())
		{
			UGameplayTagsK2Node_SwitchGameplayTag* SwitchResultTagNode = CompilerContext.SpawnIntermediateNode<UGameplayTagsK2Node_SwitchGameplayTag>(this, SourceGraph);
			SwitchResultTagNode->bHasDefaultPin = (OriginalDefaultOutExecPin != nullptr);
			SwitchResultTagNode->PinTags = ResultTags;
			SwitchResultTagNode->AllocateDefaultPins();

			bIsErrorFree &= Schema->TryCreateConnection(SwitchResultTagNode->GetSelectionPin(), CustomEventNodeParamOutPin);
			bIsErrorFree &= Schema->TryCreateConnection(SwitchResultTagNode->GetExecPin(), LastThenPinForCustomEvent);
			if (UEdGraphPin* SwitchResultDefaultExecOutPin = SwitchResultTagNode->GetDefaultPin(); (OriginalDefaultOutExecPin && SwitchResultDefaultExecOutPin))
			{
				bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*OriginalDefaultOutExecPin, *SwitchResultDefaultExecOutPin).CanSafeConnect();
			}

			for (const FName& OutputPinName : SwitchResultTagNode->PinNames)
			{
				UEdGraphPin* SwitchNodeOutputPin = SwitchResultTagNode->FindPin(OutputPinName);
				if (!bIsErrorFree || !SwitchNodeOutputPin)
					continue;

				UEdGraphPin* RelatedOriginalExecOutPin = FindPin(OutputPinName, EGPD_Output);
				bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*RelatedOriginalExecOutPin, *SwitchNodeOutputPin).CanSafeConnect();
			}
		}

		return bIsErrorFree;
	}

	return false;
}

void UK2Node_AsyncScenarioTask::LinkScenarioTaskToSpawningBlueprintNode(FString OriginNodeGuid, UAsyncScenarioTask* SpawnedTask)
{
	SpawnedTask->BlueprintNodeThatSpawnedThisTask = FGuid(OriginNodeGuid);
}
