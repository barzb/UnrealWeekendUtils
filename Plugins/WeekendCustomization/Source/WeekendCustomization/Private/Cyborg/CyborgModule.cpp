///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendCustomization UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Cyborg/CyborgModule.h"

#include "WeekendCustomization.h"
#include "Cyborg/InstallRequirements/CyborgModuleInstallRequirement_ModuleSize.h"
#include "Cyborg/InstallRequirements/CyborgModuleInstallRequirement_ModuleSocket.h"
#include "SaveGame/SaveGameSerializer.h"

UCyborgModule::UCyborgModule()
{
	InstallRequirements =
	{
		CreateDefaultSubobject<UCyborgModuleInstallRequirement_ModuleSize>("Module Size"),
		CreateDefaultSubobject<UCyborgModuleInstallRequirement_ModuleSocket>("Module Socket")
	};
}

bool UCyborgModule::IsInstalled() const
{
	return (InstallationReceipts.Num() > 0);
}

void UCyborgModule::FinishInstallation(const FCyborgModuleInstallationReceipt& Receipt)
{
	check(!IsInstalled());
	InstallationReceipts.Add(Receipt);

	for (UCyborgModuleTrait* Trait : Traits)
	{
		Trait->Activate();
	}
}

void UCyborgModule::FinishUninstallation()
{
	for (UCyborgModuleTrait* Trait : Traits)
	{
		Trait->Deactivate();
	}

	InstallationReceipts.Empty();
}

void UCyborgModule::Serialize(FArchive& Ar)
{
	Ar.UsingCustomVersion(FCyborgFrameworkVersion::GUID);

	UObject::Serialize(Ar);

	if (Ar.IsSaving() || Ar.IsLoading())
	{
		FWeekendUtilsSubobjectProxyArchive ProxyArchive(IN OUT Ar, *this);
		ProxyArchive << InstallRequirements;
		ProxyArchive << Traits;
		ProxyArchive << Config;
	}
}

bool UCyborgModule::IsTickable() const
{
	return IsInstalled();
}

void UCyborgModule::Tick(float DeltaTime)
{
	for (UCyborgModuleTrait* Trait : Traits.FilterByPredicate(&UCyborgModuleTrait::IsTickable))
	{
		Trait->Tick(DeltaTime);
	}
}

UWorld* UCyborgModule::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

TStatId UCyborgModule::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UCyborgModule, STATGROUP_Tickables);
}
