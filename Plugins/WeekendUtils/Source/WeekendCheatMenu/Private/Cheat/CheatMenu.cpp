///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
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

void UCheatMenu::HandleCheatExecuted(const ICheatMenuAction&, UWorld*, TArray<FString>)
{
	if (bShouldCloseAfterCheatExecution)
	{
		OnCloseRequested.Broadcast();
	}
}

