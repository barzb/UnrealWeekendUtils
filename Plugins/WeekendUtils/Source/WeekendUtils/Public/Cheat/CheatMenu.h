///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Cheat/SCheatMenu.h"
#include "Components/Widget.h"

#include "CheatMenu.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCheatMenuCloseRequested);

/**
 * UMG widget for displaying the cheat menu slate widget.
 */
UCLASS(ClassGroup = "Weekend Utils")
class WEEKENDUTILS_API UCheatMenu : public UWidget
{
	GENERATED_BODY()

public:
	/**
	 * Event fired after a cheat has been executed and ShouldCloseAfterCheatExecution is enabled.
	 * Request must be fulfilled by the parent widget.
	 */
	UPROPERTY(BlueprintAssignable)
	FOnCheatMenuCloseRequested OnCloseRequested;

	/** When enabled, the cheat menu will fire a close request after a cheat has been executed through the menu. */
	UFUNCTION(BlueprintCallable)
	void SetShouldCloseAfterCheatExecution(bool bShouldClose);

	/** @returns whether the cheat menu will fire a close request after a cheat has been executed through the menu. */
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool ShouldCloseAfterCheatExecution() const { return bShouldCloseAfterCheatExecution; }

protected:
	/** When enabled, the cheat menu will fire a close request after a cheat has been executed through the menu. */
	UPROPERTY(EditAnywhere)
	bool bShouldCloseAfterCheatExecution = true;

	TSharedPtr<SCheatMenu> CheatMenu = nullptr;

	// - UWidget
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif
	// --

	void HandleCheatExecuted(const ICheatCommand& CheatCommand, UWorld* World, TArray<FString> Args);
};
