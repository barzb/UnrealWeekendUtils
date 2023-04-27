/*
 * Copyright (C) 2023 by Benjamin Barz in cooperation with Nine Worlds Studios GmbH.
 *
 * This file is part of the WeekendUtils UE5 Plugin.
 *
 * Distributed under the MIT License. See accompanying file LICENSE or view online at
 * {@link https://github.com/barzb/UnrealWeekendUtils/blob/main/LICENSE}
 *
 */

using UnrealBuildTool;

public class WeekendUtilsTests : ModuleRules
{
	public WeekendUtilsTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"WeekendUtils",
			}
		);
	}
}