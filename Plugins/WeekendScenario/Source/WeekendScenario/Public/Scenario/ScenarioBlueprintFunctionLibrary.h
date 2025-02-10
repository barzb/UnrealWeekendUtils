///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendScenario UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Utils/CommonValidityEnum.h"

#include "ScenarioBlueprintFunctionLibrary.generated.h"

/**
 * #todo-docs
 */
UCLASS()
class WEEKENDSCENARIO_API UScenarioBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Scenario", meta = (ExpandEnumAsExecs = "ReturnValue"))
	static ECommonValidity GetGameScenarioTags(FGameplayTagContainer& OutTags);

	//#todo-tasks add AsyncWaitForScenarioTag task
};
