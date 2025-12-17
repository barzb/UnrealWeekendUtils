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
#include "GameplayTagContainer.h"
#include "UObject/Object.h"

#include "CyborgModuleInstaller.generated.h"

class UCyborgModule;
class UCyborgModuleInstaller;
class UCyborgSlot;
struct FCyborgModuleInstallationReceipt;

/**
 * 
 */
UCLASS(Const, BlueprintType)
class WEEKENDCUSTOMIZATION_API UCyborgModuleInstaller : public UObject
{
	GENERATED_BODY()

public:
	struct FContext
	{
		//const UObject* ContextObject;
		const UCyborgModule& Module;
		const TArray<UCyborgSlot*>& Slots;
	};

	struct FSlotEvaluation
	{
		FGameplayTagContainer FailureTags = FGameplayTagContainer();
		TMap<TObjectPtr<const UCyborgSlot>, FGameplayTagContainer> FailureTagsPerSlot = {};
	};

	struct FResult
	{
		bool bWasSuccess = false;
		TArray<TObjectPtr<const UCyborgSlot>> ResultSlots = {};
		FSlotEvaluation Evaluation = {};
	};

	virtual FResult CanInstall(const UCyborgModule& ModuleToInstall, const TArray<UCyborgSlot*>& Slots) const;
	virtual FResult CanUninstall(const UCyborgModule& ModuleToUninstall, const TArray<UCyborgSlot*>& Slots) const;
	virtual FResult Install(UCyborgModule& ModuleToInstall, TArray<UCyborgSlot*> Slots) const;
	virtual FResult Uninstall(UCyborgModule& ModuleToUninstall, const TArray<UCyborgSlot*>& Slots) const;

protected:
	virtual FCyborgModuleInstallationReceipt GenerateInstallationReceipt(const FResult& Result) const;
};
