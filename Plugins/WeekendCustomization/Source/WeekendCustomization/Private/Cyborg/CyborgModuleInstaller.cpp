///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendCustomization UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Cyborg/CyborgModuleInstaller.h"

#include "Cyborg/CyborgModule.h"
#include "Cyborg/CyborgModuleInstallRequirement.h"
#include "Cyborg/CyborgSlot.h"

UCyborgModuleInstaller::FResult UCyborgModuleInstaller::CanInstall(const UCyborgModule& ModuleToInstall, const TArray<UCyborgSlot*>& Slots) const
{
	FResult Result{.bWasSuccess = true};
	const FContext Context{.Module = ModuleToInstall, .Slots = Slots};

	for (const UCyborgModuleInstallRequirement* Requirement : ModuleToInstall.GetInstallRequirements())
	{
		Requirement->EvaluateSlots(Context, IN OUT Result.Evaluation);
	}

	for (const UCyborgSlot* SlotCandidate : Slots)
	{
		const FGameplayTagContainer* SlotEvaluation = Result.Evaluation.FailureTagsPerSlot.Find(SlotCandidate);
		if (SlotEvaluation && SlotEvaluation->Num() > 0)
			continue;

		Result.ResultSlots.Add(SlotCandidate);
	}

	for (const UCyborgModuleInstallRequirement* Requirement : ModuleToInstall.GetInstallRequirements())
	{
		Requirement->EvaluateResult(Context, IN OUT Result);
	}

	return Result;
}

UCyborgModuleInstaller::FResult UCyborgModuleInstaller::CanUninstall(const UCyborgModule& ModuleToUninstall, const TArray<UCyborgSlot*>& Slots) const
{
	FResult Result{.bWasSuccess = false};

	for (const UCyborgSlot* Slot : Slots)
	{
		if (Slot->InstalledModule != &ModuleToUninstall)
			continue;

		Result.ResultSlots.Add(Slot);
		Result.bWasSuccess = true;
	}

	return Result;
}

UCyborgModuleInstaller::FResult UCyborgModuleInstaller::Install(UCyborgModule& ModuleToInstall, TArray<UCyborgSlot*> Slots) const
{
	const FResult Result = CanInstall(ModuleToInstall, Slots);
	if (Result.bWasSuccess)
	{
		for (UCyborgSlot* SlotCandidate : Slots)
		{
			if (!Result.ResultSlots.Contains(SlotCandidate))
				continue;

			SlotCandidate->InstalledModule = &ModuleToInstall;
		}

		const FCyborgModuleInstallationReceipt Receipt = GenerateInstallationReceipt(Result);
		ModuleToInstall.FinishInstallation(Receipt);
	}
	return Result;
}

UCyborgModuleInstaller::FResult UCyborgModuleInstaller::Uninstall(UCyborgModule& ModuleToUninstall, const TArray<UCyborgSlot*>& Slots) const
{
	const FResult Result = CanUninstall(ModuleToUninstall, Slots);
	if (Result.bWasSuccess)
	{
		for (UCyborgSlot* SlotCandidate : Slots)
		{
			if (!Result.ResultSlots.Contains(SlotCandidate))
				continue;

			SlotCandidate->InstalledModule = nullptr;
		}

		ModuleToUninstall.FinishUninstallation();
	}
	return Result;
}

FCyborgModuleInstallationReceipt UCyborgModuleInstaller::GenerateInstallationReceipt(const FResult& Result) const
{
	return FCyborgModuleInstallationReceipt
	{
		.InstallerClass = GetClass(),
		.InstallDate = FDateTime::UtcNow()
	};
}
