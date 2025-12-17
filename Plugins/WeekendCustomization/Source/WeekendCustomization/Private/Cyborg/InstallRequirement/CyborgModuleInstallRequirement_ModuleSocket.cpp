///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendCustomization UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Cyborg/InstallRequirements/CyborgModuleInstallRequirement_ModuleSocket.h"

#include "Cyborg/CyborgSlot.h"

void UCyborgModuleInstallRequirement_ModuleSocket::EvaluateSlots(const FContext& InstallContext, FSlotEvaluation& InOutSlotEvaluation) const
{
	for (const UCyborgSlot* Slot : InstallContext.Slots)
	{
		if (!SocketRequirements.RequirementsMet(Slot->GetSockets()))
		{
			InOutSlotEvaluation.FailureTagsPerSlot.FindOrAdd(Slot).AddTag(FailedEvaluationTag);
		}
	}
}
