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
#include "CyborgModuleInstaller.h"
#include "UObject/Interface.h"

#include "CyborgCustomizableInterface.generated.h"

class UCyborgSlot;
class UCyborgModule;

UINTERFACE(meta = (CannotImplementInterfaceInBlueprint))
class UCyborgCustomizableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class WEEKENDCUSTOMIZATION_API ICyborgCustomizableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Customization")
	virtual TArray<UCyborgSlot*> GetCustomizationSlots() const = 0;

	UFUNCTION(BlueprintCallable, Category = "Customization")
	virtual TArray<UCyborgSlot*> GetEmptyCustomizationSlots() const;

	UFUNCTION(BlueprintCallable, Category = "Customization")
	virtual TArray<UCyborgSlot*> GetCustomizationSlotsWithInstalledModules() const;

	UFUNCTION(BlueprintCallable, Category = "Customization")
	virtual TArray<UCyborgSlot*> GetNamedCustomizationSlots(TArray<FName> SlotNames) const;

	UFUNCTION(BlueprintCallable, Category = "Customization")
	virtual TArray<UCyborgModule*> GetInstalledCustomizationModules() const;

	UFUNCTION(BlueprintCallable, Category = "Customization")
	virtual bool AreAnyCustomizationModulesInstalled() const;

	virtual UCyborgModuleInstaller::FResult CanInstallNewModule(const UCyborgModule& ModuleToInstall) const;
	virtual bool TryInstallNewModule(UCyborgModule& ModuleToInstall);

	virtual UCyborgModuleInstaller::FResult CanReplaceInstalledModule(const UCyborgModule& InstalledModule, const UCyborgModule& ReplacementModule) const;
	virtual bool TryReplaceInstalledModule(UCyborgModule& InstalledModule, UCyborgModule& ReplacementModule);

	virtual UCyborgModuleInstaller::FResult CanUninstallNewModule(const UCyborgModule& InstalledModule) const;
	virtual bool TryUninstallInstalledModule(UCyborgModule& InstalledModule);

	virtual const UCyborgModuleInstaller& GetModuleInstaller() const = 0;
};
