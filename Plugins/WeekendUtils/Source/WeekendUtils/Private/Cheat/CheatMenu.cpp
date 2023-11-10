///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz in cooperation with Aesir Interactive GmbH.
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See accompanying file LICENSE.
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Cheat/CheatMenu.h"

void UCheatMenu::SetShouldCloseAfterCheatExecution(bool bShouldClose)
{
	bShouldCloseAfterCheatExecution = bShouldClose;
}

TSharedRef<SWidget> UCheatMenu::RebuildWidget()
{
	CheatMenu = SNew(SCheatMenu)
	.NumRecentlyUsedCheatsToShow(16)
	.OnCheatExecuted_UObject(this, &ThisClass::HandleCheatExecuted);

	return CheatMenu.ToSharedRef();
}

void UCheatMenu::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	CheatMenu.Reset();
}

#if WITH_EDITOR
const FText UCheatMenu::GetPaletteCategory()
{
	return NSLOCTEXT("Weekend Utils", "Widget Palette Category", "Weekend Utils");
}
#endif

void UCheatMenu::HandleCheatExecuted(const ICheatCommand&, UWorld*, TArray<FString>)
{
	if (bShouldCloseAfterCheatExecution)
	{
		OnCloseRequested.Broadcast();
	}
}

