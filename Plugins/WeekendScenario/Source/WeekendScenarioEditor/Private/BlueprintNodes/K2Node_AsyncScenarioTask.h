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
#include "K2Node_LatentGameplayTaskCall.h"

#include "K2Node_AsyncScenarioTask.generated.h"

class FBlueprintActionDatabaseRegistrar;
class UAsyncScenarioTask;
struct FGameplayTag;

UCLASS()
class UK2Node_AsyncScenarioTask : public UK2Node_LatentGameplayTaskCall
{
	GENERATED_BODY()

public:
	UK2Node_AsyncScenarioTask(const FObjectInitializer& ObjectInitializer);

	virtual FName GetScenarioTaskClassPinName() const;
	virtual FName GetScenarioTaskNamePinName() const;

	virtual UEdGraphPin* GetScenarioTaskClassPin(const TArray<UEdGraphPin*>* InPinsToSearch = nullptr) const;
	virtual UClass* GetScenarioTaskClassToSpawn(const TArray<UEdGraphPin*>* InPinsToSearch = nullptr) const;

	TOptional<FName> GetScenarioTaskInstanceName() const;

	bool HasScenarioTaskClassPin() const;
	bool HasCompletionResult() const;
	bool ShouldBranchResultTag() const;

	UAsyncScenarioTask* FindSubtaskSpawnedByThisNode(const UAsyncScenarioTask& OwningTask) const;

protected:
	UPROPERTY()
	bool bCanBranchResultTag = true;

	UPROPERTY(EditAnywhere, Category = "Result")
	bool bBranchResultTag = false;

	UPROPERTY(EditAnywhere, Category = "Result", meta = (EditCondition = "bBranchResultTag"))
	TArray<FGameplayTag> ResultTags = {};

	UPROPERTY(EditAnywhere, Category = "Result", meta = (EditCondition = "bBranchResultTag"))
	bool bShortenResultPins = true;

	UPROPERTY(EditAnywhere, Category = "Result", meta = (EditCondition = "bBranchResultTag"))
	bool bAddBranchForOtherResults = true;

	// - UK2Node_LatentGameplayTaskCall
	virtual bool ShouldShowNodeProperties() const override { return bCanBranchResultTag; }
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual UObject* GetJumpTargetForDoubleClick() const override;
	virtual bool IsCompatibleWithGraph(UEdGraph const* TargetGraph) const override;
	virtual void ValidateNodeDuringCompilation(class FCompilerResultsLog& MessageLog) const override;
	virtual bool IsHandling(TSubclassOf<UGameplayTask> TaskClass) const override;
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	// --

	virtual bool TryExpandMulticastDelegatePropertyAsExecOutputPin(
		FMulticastDelegateProperty& DelegateProperty, UEdGraphPin* DelegatePropertyPin, const TArray<FBaseAsyncTaskHelper::FOutputPinAndLocalVariable>& TempVarOutputs,
		UEdGraphPin* ProxyObjectPin, UEdGraphPin*& InOutLastThenPin, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext);

private:
	UFUNCTION(BlueprintCallable)
	static void LinkScenarioTaskToSpawningBlueprintNode(FString OriginNodeGuid, UAsyncScenarioTask* SpawnedTask);
};
