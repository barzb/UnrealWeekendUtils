// ///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendCustomization UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Cyborg/CyborgModuleInstallRequirement.h"

#include "CyborgModuleInstallRequirement_ModuleSize.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "Install Requirement: Module Size")
class WEEKENDCUSTOMIZATION_API UCyborgModuleInstallRequirement_ModuleSize : public UCyborgModuleInstallRequirement
{
	GENERATED_BODY()

public:
	// - UCyborgInstallRequirement
	virtual void EvaluateSlots(const FContext& InstallContext, FSlotEvaluation& InOutSlotEvaluation) const override;
	virtual void EvaluateResult(const FContext& InstallContext, FResult& InOutResult) const override;
	// --

protected:
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 1, UIMin = 1))
	int32 RequiredSlotCount = 1;
};
