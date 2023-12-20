///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#if WITH_GAMEPLAY_DEBUGGER

#include "CoreMinimal.h"
#include "GameService/GameServiceBase.h"
#include "GameplayDebugger/GameplayDebuggerUtils.h"
#include "GameplayDebuggerCategory.h"

class FGameplayDebuggerCategory_GameServices final : public FGameplayDebuggerCategory
{
public:
	FGameplayDebuggerCategory_GameServices();
	GENERATE_DEBUGGER_CATEGORY(GameServices);

	// - FGameplayDebuggerCategory
	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	// --

private:
	void ToggleShowDependencies();
	static bool ShouldShowDependencies();

	TArray<FString> CollectServiceDependenciesInfo(const FGameServiceClass& ServiceClass) const;

	static FString BoolToCyanOrOrange(bool bUseCyanOverOrange);
};

#endif // WITH_GAMEPLAY_DEBUGGER