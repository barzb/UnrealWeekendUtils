///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendCustomization UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FComponentVisualizer;

DECLARE_LOG_CATEGORY_EXTERN(LogWeekendCustomization, Log, All);
DECLARE_STATS_GROUP(TEXT("WeekendCustomization"), STATGROUP_WeekendCustomization, STATCAT_Advanced);

struct FCyborgFrameworkVersion
{
	enum EType : uint8
	{
		InitialVersion = 0,

		///////////////////////////////////////////////////////////////////////////////////////
		/// New versions can be added above this marker.

		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};

	WEEKENDCUSTOMIZATION_API const static FGuid GUID;
};

class FWeekendCustomizationModule : public IModuleInterface
{
public:
	// - IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// --

protected:
	void RegisterComponentVisualizer(const FName ComponentClassName, const TSharedPtr<FComponentVisualizer> Visualizer);
	void UnregisterComponentVisualizer(const FName ComponentClassName);
};
