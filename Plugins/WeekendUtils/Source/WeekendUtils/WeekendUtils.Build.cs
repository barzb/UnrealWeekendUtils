/*
 * Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
 *
 * This file is part of the WeekendUtils UE5 Plugin.
 *
 * Distributed under the MIT License. See file: LICENSE.md
 * {@link https://github.com/barzb/UnrealWeekendUtils/blob/main/LICENSE}
 */

using UnrealBuildTool;

public class WeekendUtils : ModuleRules
{
	public WeekendUtils(ReadOnlyTargetRules Target) : base(Target)
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
				"EnhancedInput",
				"GameplayAbilities",
				"GameplayTasks",
				"ModelViewViewModel",
				"Slate", "SlateCore",
				"StructUtils",
				"UMG",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"AssetRegistry",
				"CoreUObject",
				"Engine",
				"GameFeatures",
				"InputCore",
				"Projects",
			}
		);

		if (Target.Type == TargetType.Editor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"AssetTools",
					"Blutility",
					"UnrealEd",
				}
			);
		}
	}
}
