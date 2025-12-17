///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendCustomization UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Cyborg/CyborgSlot.h"

#include "Cyborg/CyborgModule.h"

FString UCyborgSlot::GetDebugInfo() const
{
	FString DebugInfo;
	if (Sockets.Num() > 0)
	{
		DebugInfo += "Sockets: " + Sockets.ToString() + " | ";
	}

	DebugInfo += "InstalledModule: " + GetNameSafe(InstalledModule);
	return DebugInfo;
}
