/*
 * Copyright (C) 2023 by Benjamin Barz in cooperation with Nine Worlds Studios GmbH.
 * 
 * This file is part of the WeekendUtils UE5 Plugin.
 * 
 * Distributed under the MIT License. See accompanying file LICENSE or view online at
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
				"Slate",
				"SlateCore",
				"StructUtils",
				// ... add other public dependencies that you statically link with here ...
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"InputCore",
				// ... add private dependencies that you statically link with here ...
			}
		);
	}
}