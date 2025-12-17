///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendCustomization UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Cyborg/CyborgModuleInstaller.h"
#include "UObject/Object.h"

#include "CyborgModuleInstallRequirement.generated.h"

/**
 * 
 */
UCLASS(Abstract, Const, BlueprintType, EditInlineNew, DefaultToInstanced, Within = CyborgModule)
class WEEKENDCUSTOMIZATION_API UCyborgModuleInstallRequirement : public UObject
{
	GENERATED_BODY()

public:
	using FResult = UCyborgModuleInstaller::FResult;
	using FContext = UCyborgModuleInstaller::FContext;
	using FSlotEvaluation = UCyborgModuleInstaller::FSlotEvaluation;

	virtual void EvaluateSlots(const FContext& InstallContext, FSlotEvaluation& InOutSlotEvaluation) const {}
	virtual void EvaluateResult(const FContext& InstallContext, FResult& InOutResult) const {}

protected:
	UPROPERTY(EditAnywhere, meta = (Categories = "Customization.ModuleInstaller.Evaluation"))
	FGameplayTag FailedEvaluationTag = FGameplayTag();
};
