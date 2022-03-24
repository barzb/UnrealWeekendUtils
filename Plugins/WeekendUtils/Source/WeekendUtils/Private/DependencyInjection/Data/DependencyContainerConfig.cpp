// (c) by Benjamin Barz

#include "DependencyInjection/Data/DependencyContainerConfig.h"

#if WITH_EDITOR
EDataValidationResult UDependencyContainerConfig::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(ValidationErrors), EDataValidationResult::Valid);

	int32 i = 0;
	for (const TPair<TSubclassOf<UObject>, FDependencyList>& Itr : DependenciesForClasses)
	{
		const TSubclassOf<UObject> ConfigClass = Itr.Key;
		if (!ConfigClass)
		{
			Result = EDataValidationResult::Invalid;
			ValidationErrors.Add(FText::FromString(FString::Printf(TEXT("Invalid class at index %d"), i)));
			continue;
		}
		for (const TSubclassOf<UObject> Dependency : Itr.Value.Entries)
		{
			if (!Dependency)
			{
				Result = EDataValidationResult::Invalid;
				ValidationErrors.Add(FText::FromString(FString::Printf(TEXT("Invalid dependency class for %s at index %d"), *ConfigClass->GetName(), i)));
			}
		}
		i++;
	}
	return Result;
}
#endif
