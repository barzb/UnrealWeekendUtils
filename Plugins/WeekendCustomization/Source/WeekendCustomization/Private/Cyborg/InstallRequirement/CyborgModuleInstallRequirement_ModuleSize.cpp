///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendCustomization UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Cyborg/InstallRequirements/CyborgModuleInstallRequirement_ModuleSize.h"

#include "Algo/Count.h"
#include "Cyborg/CyborgSlot.h"

void UCyborgModuleInstallRequirement_ModuleSize::EvaluateSlots(const FContext& InstallContext, FSlotEvaluation& InOutSlotEvaluation) const
{
	checkf(RequiredSlotCount > 0, TEXT("RequiredSlotCount must be > 0 on %s"), *GetPathName());
	const int32 NumFreeSlots = Algo::CountIf(InstallContext.Slots, &UCyborgSlot::IsAvailableForInstallation);
	if (NumFreeSlots < RequiredSlotCount)
	{
		InOutSlotEvaluation.FailureTags.AddTag(FailedEvaluationTag);
	}
}

void UCyborgModuleInstallRequirement_ModuleSize::EvaluateResult(const FContext& InstallContext, FResult& InOutResult) const
{
	InOutResult.bWasSuccess &= !InOutResult.Evaluation.FailureTags.HasTag(FailedEvaluationTag);
	InOutResult.bWasSuccess &= (InOutResult.ResultSlots.Num() >= RequiredSlotCount);
	InOutResult.ResultSlots.SetNum(InOutResult.bWasSuccess ? RequiredSlotCount : 0);
}
