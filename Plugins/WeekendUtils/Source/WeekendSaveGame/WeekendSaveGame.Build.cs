/*
 * Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
 *
 * This file is part of the WeekendUtils UE5 Plugin.
 *
 * Distributed under the MIT License. See file: LICENSE.md
 * {@link https://github.com/barzb/UnrealWeekendUtils/blob/main/LICENSE}
 */

using UnrealBuildTool;

public class WeekendSaveGame : ModuleRules
{
	public WeekendSaveGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"ModelViewViewModel",
				"UMG",
				"WeekendGameService",
				"WeekendUtils",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Blutility",
				"CoreUObject",
				"DeveloperSettings",
				"Engine",
				"WeekendCheatMenu",
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