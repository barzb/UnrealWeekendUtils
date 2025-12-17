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
#include "GameplayEffectTypes.h"
#include "Cyborg/CyborgModuleInstallRequirement.h"

#include "CyborgModuleInstallRequirement_ModuleSocket.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "Install Requirement: Module Socket")
class WEEKENDCUSTOMIZATION_API UCyborgModuleInstallRequirement_ModuleSocket : public UCyborgModuleInstallRequirement
{
	GENERATED_BODY()

public:
	// - UCyborgInstallRequirement
	virtual void EvaluateSlots(const FContext& InstallContext, FSlotEvaluation& InOutSlotEvaluation) const override;
	// --

protected:
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, meta = (Categories = "Customization.Socket"))
	FGameplayTagRequirements SocketRequirements = FGameplayTagRequirements();
};
