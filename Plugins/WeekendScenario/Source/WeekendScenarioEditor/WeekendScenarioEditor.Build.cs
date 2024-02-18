/*
 * Copyright (C) 2024 by Benjamin Barz and contributors. See file: CREDITS.md
 *
 * This file is part of the WeekendScenario UE5 Plugin.
 *
 * Distributed under the MIT License. See file: LICENSE.md
 * {@link https://github.com/barzb/UnrealWeekendUtils/blob/main/LICENSE}
 */

using UnrealBuildTool;

public class WeekendScenarioEditor : ModuleRules
{
	public WeekendScenarioEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		//PublicIncludePaths.AddRange();
		// ... add public include paths required here ...

		//PrivateIncludePaths.AddRange();
		// ... add other private include paths required here ...

		//DynamicallyLoadedModuleNames.AddRange();
		// ... add any modules that your module loads dynamically here ...

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"GameplayTasks",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"BlueprintGraph",
				"Core", "CoreUObject",
				"Engine",
				"GameplayTags", "GameplayTagsEditor",
				"GameplayTasks", "GameplayTasksEditor",
				"GraphEditor",
				"Kismet", "KismetCompiler",
				"Slate", "SlateCore",
				"UnrealEd",
				"WeekendScenario",
				"WeekendUtils",
			}
		);
	}
}