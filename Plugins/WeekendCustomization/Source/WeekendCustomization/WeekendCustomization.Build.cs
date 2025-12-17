/*
 * Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
 *
 * This file is part of the WeekendCustomization UE5 Plugin.
 *
 * Distributed under the MIT License. See file: LICENSE.md
 * {@link https://github.com/barzb/UnrealWeekendUtils/blob/main/LICENSE}
 */

using UnrealBuildTool;

public class WeekendCustomization : ModuleRules
{
	public WeekendCustomization(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		SetupGameplayDebuggerSupport(Target);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GameFeatures",
				"GameplayAbilities",
				"GameplayTags",
				"ModelViewViewModel",
				"WeekendGameService",
				"WeekendUtils",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"WeekendSaveGame",
			}
		);

		if (Target.Type == TargetType.Editor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"AssetTools",
					"UnrealEd",
				}
			);
		}
	}
}