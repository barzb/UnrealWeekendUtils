///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#if WITH_GAMEPLAY_DEBUGGER

#include "GameplayDebugger/Extensions/GameplayDebuggerExtension_ToggleUiVisibility.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

FGameplayDebuggerExtension_ToggleUiVisibility::FGameplayDebuggerExtension_ToggleUiVisibility()
{
	const FGameplayDebuggerInputHandlerConfig KeyConfig(TEXT("Trigger"), EKeys::H.GetFName(), FGameplayDebuggerInputModifier::Ctrl);
	bHasInputBinding = BindKeyPress(KeyConfig, this, &FGameplayDebuggerExtension_ToggleUiVisibility::ToggleUiVisibility);
}

FString FGameplayDebuggerExtension_ToggleUiVisibility::GetDescription() const
{
	if (!bHasInputBinding)
		return FString();

	return FString::Printf(TEXT("{%s}%s:{%s}Toggle UI Visibility"),
		*FGameplayDebuggerCanvasStrings::ColorNameInput,
		*GetInputHandlerDescription(0),
		(bTurnVisibleNext ? *FGameplayDebuggerCanvasStrings::ColorNameEnabled : *FGameplayDebuggerCanvasStrings::ColorNameDisabled));
}

UClass* FGameplayDebuggerExtension_ToggleUiVisibility::GetTopLevelWidgetClass()
{
	UClass* Class = nullptr;

	static const FString ConfigSection = "GameplayDebugger";
	static const FString ConfigKey = "TopLevelWidgetClass";
	if (FString OverrideClassName; GConfig->GetString(*ConfigSection, *ConfigKey, OUT OverrideClassName, GEngineIni))
	{
		Class = FSoftClassPath(OverrideClassName).TryLoadClass<UUserWidget>();
	}

	return (Class != nullptr) ? Class : UUserWidget::StaticClass();
}

void FGameplayDebuggerExtension_ToggleUiVisibility::ToggleUiVisibility()
{
	TArray<UUserWidget*> TopLevelWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetPlayerController(), OUT TopLevelWidgets, GetTopLevelWidgetClass(), true);

	static TMap<TWeakObjectPtr<UUserWidget>, ESlateVisibility> HiddenWidgets;
	for (UUserWidget* TopLevelWidget : TopLevelWidgets)
	{
		if (bTurnVisibleNext)
		{
			if (!HiddenWidgets.Contains(TopLevelWidget))
				continue;

			TopLevelWidget->SetVisibility(HiddenWidgets[TopLevelWidget]);
		}
		else
		{
			HiddenWidgets.FindOrAdd(TopLevelWidget, TopLevelWidget->GetVisibility());
			TopLevelWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	bTurnVisibleNext = !bTurnVisibleNext;
	if (!bTurnVisibleNext)
	{
		HiddenWidgets.Empty();
	}
}

#endif
