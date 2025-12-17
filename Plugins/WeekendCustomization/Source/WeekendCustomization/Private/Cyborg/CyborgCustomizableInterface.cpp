///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendCustomization UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Cyborg/CyborgCustomizableInterface.h"

#include "Cyborg/CyborgSlot.h"

TArray<UCyborgSlot*> ICyborgCustomizableInterface::GetEmptyCustomizationSlots() const
{
	return GetCustomizationSlots().FilterByPredicate(&UCyborgSlot::IsAvailableForInstallation);
}

TArray<UCyborgSlot*> ICyborgCustomizableInterface::GetCustomizationSlotsWithInstalledModules() const
{
	return GetCustomizationSlots().FilterByPredicate(&UCyborgSlot::IsAnyModuleInstalled);
}

TArray<UCyborgSlot*> ICyborgCustomizableInterface::GetNamedCustomizationSlots(TArray<FName> SlotNames) const
{
	return GetCustomizationSlots().FilterByPredicate([SlotNames](const UCyborgSlot* Slot) { return SlotNames.Contains(Slot->GetSlotName()); });
}

TArray<UCyborgModule*> ICyborgCustomizableInterface::GetInstalledCustomizationModules() const
{
	TArray<UCyborgModule*> Modules{};
	Algo::Transform(GetCustomizationSlotsWithInstalledModules(), OUT Modules, &UCyborgSlot::GetInstalledModule);
	return Modules;
}

bool ICyborgCustomizableInterface::AreAnyCustomizationModulesInstalled() const
{
	return GetInstalledCustomizationModules().Num() > 0;
}

UCyborgModuleInstaller::FResult ICyborgCustomizableInterface::CanInstallNewModule(const UCyborgModule& ModuleToInstall) const
{
	const UCyborgModuleInstaller& Installer = GetModuleInstaller();
	return Installer.CanInstall(ModuleToInstall, GetEmptyCustomizationSlots());
}

bool ICyborgCustomizableInterface::TryInstallNewModule(UCyborgModule& ModuleToInstall)
{
	const UCyborgModuleInstaller& Installer = GetModuleInstaller();
	return Installer.Install(ModuleToInstall, GetEmptyCustomizationSlots()).bWasSuccess;
}

UCyborgModuleInstaller::FResult ICyborgCustomizableInterface::CanReplaceInstalledModule(const UCyborgModule& InstalledModule, const UCyborgModule& ReplacementModule) const
{
	UCyborgModuleInstaller::FResult ReplacementResult{.bWasSuccess = false};

	using UFakeSlot = UCyborgSlot;
	using URealSlot = UCyborgSlot;
	TMap<UFakeSlot*, const URealSlot*> RealSlotsForFakeReplacements = {};
	TArray<UCyborgSlot*> SlotsToTestInstallation = GetEmptyCustomizationSlots();
	const UCyborgModuleInstaller& Installer = GetModuleInstaller();

	// Pretend the installed module was uninstalled:
	{
		const UCyborgModuleInstaller::FResult UninstallResult = Installer.CanUninstall(InstalledModule, GetCustomizationSlotsWithInstalledModules());
		if (!UninstallResult.bWasSuccess)
			return ReplacementResult;

		for (const URealSlot* RealSlot : UninstallResult.ResultSlots)
		{
			if (RealSlot->IsAvailableForInstallation())
				continue;

			UFakeSlot* FakeEmptySlot = NewObject<UFakeSlot>(RealSlot->GetOuter(), RealSlot->GetClass(),
															*(RealSlot->GetName() + "_temp"), RF_NoFlags, const_cast<UCyborgSlot*>(RealSlot));
			Installer.Uninstall(*FakeEmptySlot->GetInstalledModule(), {FakeEmptySlot});

			SlotsToTestInstallation.Add(FakeEmptySlot);
			RealSlotsForFakeReplacements.Add(FakeEmptySlot, RealSlot);
		}
	}

	// Pretend the replacement module was installed afterward:
	{
		ReplacementResult = Installer.CanInstall(ReplacementModule, SlotsToTestInstallation);

		for (int32 i = 0; i < ReplacementResult.ResultSlots.Num(); ++i)
		{
			const UCyborgSlot* ResultSlot = ReplacementResult.ResultSlots[i].Get();
			if (!RealSlotsForFakeReplacements.Contains(ResultSlot))
				continue;

			ReplacementResult.ResultSlots[i] = RealSlotsForFakeReplacements[ResultSlot];
		}

		for (auto Itr = ReplacementResult.Evaluation.FailureTagsPerSlot.CreateIterator(); Itr; ++Itr)
		{
			const UCyborgSlot* ResultSlot = Itr->Key;
			if (!RealSlotsForFakeReplacements.Contains(ResultSlot))
				continue;

			Itr->Key = RealSlotsForFakeReplacements[ResultSlot];
		}
	}

	return ReplacementResult;
}

bool ICyborgCustomizableInterface::TryReplaceInstalledModule(UCyborgModule& InstalledModule, UCyborgModule& ReplacementModule)
{
	UCyborgModuleInstaller::FResult ReplacementResult = CanReplaceInstalledModule(InstalledModule, ReplacementModule);
	if (!ReplacementResult.bWasSuccess)
		return false;

	const UCyborgModuleInstaller& Installer = GetModuleInstaller();

	const UCyborgModuleInstaller::FResult UninstallResult = Installer.Uninstall(InstalledModule, GetCustomizationSlotsWithInstalledModules());
	check(UninstallResult.bWasSuccess);

	const UCyborgModuleInstaller::FResult InstallResult = Installer.Install(ReplacementModule, GetEmptyCustomizationSlots());
	check(InstallResult.bWasSuccess)

	return true;
}

UCyborgModuleInstaller::FResult ICyborgCustomizableInterface::CanUninstallNewModule(const UCyborgModule& InstalledModule) const
{
	const UCyborgModuleInstaller& Installer = GetModuleInstaller();
	return Installer.CanUninstall(InstalledModule, GetCustomizationSlotsWithInstalledModules());
}

bool ICyborgCustomizableInterface::TryUninstallInstalledModule(UCyborgModule& InstalledModule)
{
	const UCyborgModuleInstaller& Installer = GetModuleInstaller();
	return Installer.Uninstall(InstalledModule, GetCustomizationSlotsWithInstalledModules()).bWasSuccess;
}
