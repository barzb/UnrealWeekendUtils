// (c) by Benjamin Barz

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"

#include "ScenarioProjectSettings.generated.h"

/**
 * #todo-docs
 */
UCLASS(Config = "Scenario", DefaultConfig, meta = (DisplayName = "Weekend Scenarios"))
class WEEKENDSCENARIO_API UScenarioProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "General") // ?
	FGameplayTag GenericScenarioTaskSuccessTag = FGameplayTag();

	UPROPERTY(Config, EditAnywhere, Category = "General") // ?
	FGameplayTag GenericScenarioTaskFailureTag = FGameplayTag();
};
