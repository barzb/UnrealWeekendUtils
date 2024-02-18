/*
 * Copyright (C) 2024 by Benjamin Barz and contributors. See file: CREDITS.md
 *
 * This file is part of the WeekendScenario UE5 Plugin.
 *
 * Distributed under the MIT License. See file: LICENSE.md
 * {@link https://github.com/barzb/UnrealWeekendUtils/blob/main/LICENSE}
 */

using UnrealBuildTool;

public class WeekendScenario : ModuleRules
{
	public WeekendScenario(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		SetupGameplayDebuggerSupport(Target);

		//PublicIncludePaths.AddRange();
		// ... add public include paths required here ...

		//PrivateIncludePaths.AddRange();
		// ... add other private include paths required here ...

		//DynamicallyLoadedModuleNames.AddRange();
		// ... add any modules that your module loads dynamically here ...

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"DeveloperSettings",
				"GameplayTags",
				"GameplayTasks",
				"WeekendUtils",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate", "SlateCore",
			}
		);
	}
}